#include "../console.h"
#include "../file_utils.h"
#include "../memory.h"
#include "../sleep.h"
#include "../client/check_root.h"

#include <math.h>   // fabs
#include <errno.h>  // errno
#include <stdio.h>  // printf, fprintf, fopen, fclose
#include <stdlib.h> // strtoll
#include <string.h> // strcmp

static const struct cli99_Option Monitor_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-r|--report",      Option_Report,        cli99_RequiredArgument},
  {"-c|--clearly",     Option_Clearly,       cli99_NoArgument      },
  {"-d|--decimal",     Option_Decimal,       cli99_NoArgument      },
  {"-t|--timespan",    Option_Timespan,      cli99_RequiredArgument},
  {"-i|--interval",    Option_Interval,      cli99_RequiredArgument},
  {"-C|--cpu",         Option_Cpu,           cli99_NoArgument,     },
  {"-G|--gpu",         Option_Gpu,           cli99_NoArgument,     },
  cli99_Options_End()
};

static const struct cli99_Option Graph_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-d|--decimal",     Option_Decimal,       cli99_NoArgument      },
  {"--register-color", Option_RegisterColor, cli99_RequiredArgument},
  {"--cpu-color",      Option_CpuColor,      cli99_RequiredArgument},
  {"--gpu-color",      Option_GpuColor,      cli99_RequiredArgument},
  {"file",             Option_File,          cli99_NormalPositional},
  cli99_Options_End()
};

static const struct cli99_Option Evaluate_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-d|--decimal",     Option_Decimal,       cli99_NoArgument      },
  {"file",             Option_File,          cli99_NormalPositional},
  cli99_Options_End()
};

static bool RegisterHasChanged(RegisterReadings readings[], int size, int register_) {
  if (size == 0)
    return false;

  const uint8_t first = readings[0].readings[register_];
  for (range(int, i, 1, size))
    if (first != readings[i].readings[register_])
      return true;
  return false;
}

static void PrintMonitor(RegisterReadings readings[], int size, bool cpu, bool gpu) {
  printf(CONSOLE_CLEAR);

  for (int register_ = 0; register_ < REGISTERS_SIZE; ++register_) {
    if (! RegisterHasChanged(readings, size, register_))
      continue;

    printf(CONSOLE_GREEN "0x%.2X:", register_);
    uint8_t byte = readings[0].readings[register_];
    for (range(int, i, MAX(size - 24, 0), size)) {
      const uint8_t diff = byte - readings[i].readings[register_];
      byte = readings[i].readings[register_];
      if (diff)
        printf(CONSOLE_BOLD_BLUE " %.2X", byte);
      else
        printf(CONSOLE_BOLD_WHITE " %.2X", byte);
    }
    printf("\n");
  }

  if (cpu) {
    printf(CONSOLE_GREEN "@CPU:");
    for (range(int, i, MAX(size - 24, 0), size))
      printf(CONSOLE_BOLD_WHITE " %d", (int) readings[i].cpu_temp);
    printf("\n");
  }

  if (gpu) {
    printf(CONSOLE_GREEN "@GPU:");
    for (range(int, i, MAX(size - 24, 0), size))
      printf(CONSOLE_BOLD_WHITE " %d", (int) readings[i].gpu_temp);
    printf("\n");
  }
}

static void WriteMonitorReport(RegisterReadings readings[], int size, bool cpu, bool gpu, FILE* fh) {
  for (int register_ = 0; register_ < REGISTERS_SIZE; ++register_) {
    if (! RegisterHasChanged(readings, size, register_))
      continue;

    fprintf(fh, "%.2X", register_);
    for (range(int, i, 0, size)) {
      if (options.clearly &&
          i > 0 &&
          readings[i].readings[register_] == readings[i - 1].readings[register_])
      {
        continue;
      }

      if (options.decimal)
        fprintf(fh, ",%d", readings[i].readings[register_]);
      else
        fprintf(fh, ",%.2X", readings[i].readings[register_]);
    }
    fprintf(fh, "\n");
  }

  if (cpu) {
    fprintf(fh, "@CPU");
    for (range(int, i, 0, size)) {
      fprintf(fh, ",%.2f", readings[i].cpu_temp);
    }
    fprintf(fh, "\n");
  }

  if (gpu) {
    fprintf(fh, "@GPU");
    for (range(int, i, 0, size)) {
      fprintf(fh, ",%.2f", readings[i].gpu_temp);
    }
    fprintf(fh, "\n");
  }
}

static int Monitor(void) {
  Error e = err_success();
  int max_loops = INT_MAX;
  FS_TemperatureSource_References cpu_sensors = {0};
  FS_TemperatureSource_References gpu_sensors = {0};

  check_root();
  Initialize_EC();

  if (options.timespan)
    max_loops = (int) ((float) options.timespan / options.interval);

  if (max_loops > ARRAY_SSIZE(Registers_Log))
    max_loops = ARRAY_SSIZE(Registers_Log);

  if (options.cpu || options.gpu) {
    e = FS_Sensors_Init(options.gpu);
    if (e)
      goto error;
  }

  if (options.cpu) {
    e = FS_TemperatureSources_AddTemperatureSources(&cpu_sensors, "@CPU");
    if (e)
      goto error;
  }

  if (options.gpu) {
    e = FS_TemperatureSources_AddTemperatureSources(&gpu_sensors, "@GPU");
    if (e)
      goto error;
  }

  RegisterReadings* regs = Registers_Log;
  int loops = 0;
  while (loops < max_loops && !quit) {
    e = Registers_FromEC(regs + loops);
    if (e)
      goto error;

    if (options.cpu) {
      e = FS_TemperatureSources_GetTemperature(&cpu_sensors, TemperatureAlgorithmType_Average, &regs[loops].cpu_temp);
      if (e)
        goto error;
    }

    if (options.gpu) {
      e = FS_TemperatureSources_GetTemperature(&gpu_sensors, TemperatureAlgorithmType_Average, &regs[loops].gpu_temp);
      if (e)
        goto error;
    }

    // Increment `loops` here, because PrintMonitor() expects size, not index.
    loops++;
    PrintMonitor(regs, loops, options.cpu, options.gpu);
    sleep_ms((unsigned) (options.interval * 1000.0f));
  }

  if (options.report) {
    FILE* fh = fopen(options.report, "w");
    if (! fh) {
      e = err_stringf("%s: %s", options.report, strerror(errno));
      goto error;
    }
    WriteMonitorReport(regs, loops, options.cpu, options.gpu, fh);
    fclose(fh);
  }

error:
  if (e)
    Log_Error("%s", err_print_all(e));

  FS_Sensors_Cleanup();

  return e ? NBFC_EXIT_FAILURE : NBFC_EXIT_SUCCESS;
}

static int Graph(void) {
  size_t argc = 0;
  char* argv[16] = {0};
  argv[argc++] = Mem_Strdup(NBFC_MAKE_GRAPH_SCRIPT);
  argv[argc++] = Mem_Strdup(options.file);
  argv[argc++] = Mem_Strdup("--register-color");
  argv[argc++] = Mem_Strdup(options.register_color ? options.register_color : "blue");
  argv[argc++] = Mem_Strdup("--cpu-color");
  argv[argc++] = Mem_Strdup(options.cpu_color ? options.cpu_color : "red");
  argv[argc++] = Mem_Strdup("--gpu-color");
  argv[argc++] = Mem_Strdup(options.gpu_color ? options.gpu_color : "magenta");

  if (options.decimal)
    argv[argc++] = Mem_Strdup("-d");

  execv(NBFC_MAKE_GRAPH_SCRIPT_FILE, argv);

  // We don't free() here, because `Graph()` is a one shot operation.
  Log_Error("execv(): %s", strerror(errno));
  return NBFC_EXIT_FAILURE;
}

struct Evaluate_CsvData {
  array_of(uint8_t) register_readings[REGISTERS_SIZE];
  array_of(float) cpu_readings;
  array_of(float) gpu_readings;
};
typedef struct Evaluate_CsvData Evaluate_CsvData;

static void Evaluate_CsvData_Free(Evaluate_CsvData* data) {
  for (int i = 0; i < REGISTERS_SIZE; ++i)
    Mem_Free(data->register_readings[i].data);
  Mem_Free(data->cpu_readings.data);
  Mem_Free(data->gpu_readings.data);
  memset(data, 0, sizeof(Evaluate_CsvData));
}

/*
 * Split text by comma.
 */
static void Evaluate_SplitFields(array_of(str)* out, char* line)
{
  out->size = 0;
  out->data = NULL;

  char* field = line;

  for (char* p = line;; ++p) {
    if (*p == ',' || *p == '\0') {
      char delimiter = *p;
      *p = '\0';

      array_realloc(str, *out, out->size + 1);
      out->data[out->size++] = Mem_Strdup(field);

      if (delimiter == '\0')
        break;

      field = p + 1;
    }
  }
}

/*
 * Split text by newline.
 */
static void Evaluate_SplitLines(array_of(str)* out, char* content)
{
  out->size = 0;
  out->data = NULL;

  char* line = content;

  for (char* p = content;; ++p) {
    if (*p == '\n' || *p == '\0') {
      char delimiter = *p;
      *p = '\0';

      array_realloc(str, *out, out->size + 1);
      out->data[out->size++] = Mem_Strdup(line);

      if (delimiter == '\0')
        break;

      line = p + 1;
    }
  }
}

/*
 * Parse a uint8_t from a string and return the value.
 *
 * On success, `errno` is 0.
 * On failure, `errno` contains the error.
 */
static uint8_t Evaluate_ParseUInt8(const char* s, int base) {
  errno = 0;
  char* end;
  int64_t val = strtoll(s, &end, base);

  if (errno)
    (void) 0; // strtoll set errno
  else if (*s == '\0' || *end)
    errno = EINVAL;
  else if (val < 0 || val > UINT8_MAX)
    errno = ERANGE;

  return (uint8_t) val;
}

static Error Evaluate_ArrayOfStringToArrayOfFloat(array_of(str)* strings, array_of(float)* out) {
  out->size = 0;
  out->data = NULL;
  const char* err;

  array_calloc(float, *out, strings->size);

  for_each_array(str*, string, *strings) {
    out->data[out->size++] = (float) parse_double(*string, -FLT_MAX, FLT_MAX, &err);
    if (err)
      return err_stringf("%s: %s", *string, err);
  }

  return err_success();
}

static Error Evaluate_ArrayOfStringToArrayOfUInt8(array_of(str)* strings, int base, array_of(uint8_t)* out) {
  out->size = 0;
  out->data = NULL;

  array_calloc(uint8_t, *out, strings->size);

  for_each_array(str*, string, *strings) {
    out->data[out->size++] = (uint8_t) Evaluate_ParseUInt8(*string, base);
    if (errno)
      return err_stdlib(*string);
  }

  return err_success();
}

static Error Evaluate_CsvData_FromFile(Evaluate_CsvData* data, const char* file) {
  Error e = err_success();
  FileResult res;
  char* content = NULL;
  array_of(str) lines = {0};
  array_of(str) fields = {0};

  memset(data, 0, sizeof(Evaluate_CsvData));

  res = File_ReadDynamic(&content, file);
  if (! res.ok) {
    e = err_stdlib(NULL);
    goto error;
  }

  Evaluate_SplitLines(&lines, content);
  if (lines.size == 0) {
    e = err_string("No lines found");
    goto error;
  }

  for_each_array(str*, line, lines) {
    if (**line == '\0')
      continue;

    Evaluate_SplitFields(&fields, (char*) *line);
    if (fields.size == 0)
      continue;

    if (! strcmp(fields.data[0], "@CPU")) {
      array_of(str) readings;
      readings.data = fields.data + 1;
      readings.size = fields.size - 1;
      e = Evaluate_ArrayOfStringToArrayOfFloat(&readings, &data->cpu_readings);
      if (e) {
        e = err_chain_string(e, "@CPU");
        goto error;
      }
    }
    else if (! strcmp(fields.data[0], "@GPU")) {
      array_of(str) readings;
      readings.data = fields.data + 1;
      readings.size = fields.size - 1;
      e = Evaluate_ArrayOfStringToArrayOfFloat(&readings, &data->gpu_readings);
      if (e) {
        e = err_chain_string(e, "@GPU");
        goto error;
      }
    }
    else {
      uint8_t register_ = Evaluate_ParseUInt8(fields.data[0], 16);
      if (errno) {
        e = err_stdlib(fields.data[0]);
        goto error;
      }

      array_of(str) readings;
      readings.data = fields.data + 1;
      readings.size = fields.size - 1;

      const int base = options.decimal ? 10 : 16;
      e = Evaluate_ArrayOfStringToArrayOfUInt8(&readings, base, &data->register_readings[register_]);
      if (e) {
        goto error;
      }
    }

    // Clear array
    for_each_array(str*, field, fields)
      Mem_Free((char*) *field);
    Mem_Free(fields.data);
    memset(&fields, 0, sizeof(fields));
  }

error:
  if (e)
    Evaluate_CsvData_Free(data);

  for_each_array(str*, line, lines)
    Mem_Free((char*) *line);
  Mem_Free(lines.data);

  for_each_array(str*, field, fields)
    Mem_Free((char*) *field);
  Mem_Free(fields.data);

  Mem_Free(content);
  return e;
}

static void Evaluate_ArrayOfFloatToArrayOfDouble(array_of(float)* in, array_of(double)* out) {
  out->size = 0;
  out->data = NULL;

  array_calloc(double, *out, in->size);

  for_each_array(float*, val, *in)
    out->data[out->size++] = *val;
}

static void Evaluate_ArrayOfUInt8ToArrayOfDouble(array_of(uint8_t)* in, array_of(double)* out) {
  out->size = 0;
  out->data = NULL;

  array_calloc(double, *out, in->size);

  for_each_array(uint8_t*, val, *in)
    out->data[out->size++] = *val;
}

static void Evaluate_Print(
  const char* sensor_name,
  array_of(float)* sensor_readings,
  Evaluate_CsvData* data)
{
  double result[REGISTERS_SIZE] = {0};
  array_of(double) temperature_readings = {0};
  array_of(double) register_readings = {0};

  Evaluate_ArrayOfFloatToArrayOfDouble(sensor_readings, &temperature_readings);

  for (int register_ = 0; register_ < 256; ++register_) {
    if (data->register_readings[register_].size == 0)
      continue;

    Evaluate_ArrayOfUInt8ToArrayOfDouble(&data->register_readings[register_], &register_readings);

    result[register_] = fabs(spearman_correlation(temperature_readings.data, register_readings.data, register_readings.size));

    Mem_Free(register_readings.data);
  }

  Mem_Free(temperature_readings.data);

  printf("%s\n", sensor_name);
  for (int register_ = 0; register_ < 256; ++register_) {
    if (result[register_] > 0.2)
      printf("  0x%.2X: %.2f%%\n", register_, result[register_] * 100.0);
  }
}

static int Evaluate(void) {
  Error e;
  Evaluate_CsvData data = {0};
  int ret = NBFC_EXIT_FAILURE;

  const char* infile = options.file;
  if (! strcmp(infile, "-"))
    infile = "/dev/stdin";

  e = Evaluate_CsvData_FromFile(&data, infile);
  if (e) {
    Log_Error("%s: %s", infile, err_print_all(e));
    goto error;
  }

  if (data.cpu_readings.size == 0 && data.gpu_readings.size == 0) {
    Log_Error("Input file does not have CPU/GPU readings");
    goto error;
  }

  if (data.cpu_readings.size != 0 && data.gpu_readings.size != 0 &&
      data.cpu_readings.size != data.gpu_readings.size)
  {
    Log_Error("Number of CPU readings != GPU readings");
    goto error;
  }

  array_size_t size = MAX(data.cpu_readings.size, data.gpu_readings.size);

  for (int register_ = 0; register_ < REGISTERS_SIZE; ++register_) {
    if (data.register_readings[register_].size != 0 &&
        data.register_readings[register_].size != size)
    {
      Log_Error("Number of readings of register %.2X does not match CPU/GPU readings", register_);
      Log_Error("Evaluate does not support reports made with `ec_probe monitor -c|--clearly`");
      goto error;
    }
  }

  if (data.cpu_readings.size)
    Evaluate_Print("@CPU", &data.cpu_readings, &data);

  if (data.gpu_readings.size)
    Evaluate_Print("@GPU", &data.gpu_readings, &data);

  fprintf(stderr,
    "\n"
    "It is highly recommended to verify the reported registers,\n"
    "for example using `nbfc acpi-dump`. Do not blindly trust these results!\n");

  ret = NBFC_EXIT_SUCCESS;

error:
  Evaluate_CsvData_Free(&data);
  return ret;
}
