#include <fcntl.h>  // open, O_WRONLY, O_CREAT, O_TRUNC
#include <float.h>  // FLT_MAX
#include <math.h>   // fabs, ceilf
#include <unistd.h> // close
#include <string.h> // strcmp, memset
#include <sys/stat.h> // S_IRUSR, S_IWUSR, S_IRGRP...

#include "check_root.h"
#include "client_global.h"
#include "config_files.h"
#include "spearman.h"

#include "../acpi_call.h"
#include "../ec.h"
#include "../ec_cache.h"
#include "../error.h"
#include "../log.h"
#include "../nbfc.h"
#include "../macros.h"
#include "../memory.h"
#include "../help/client.help.h"
#include "../nxjson_utils.h"
#include "../nxjson_write.h"
#include "../file_utils.h"
#include "../fs_sensors.h"
#include "../fan.h"
#include "../model_config.h"
#include "../sleep.h"
#include "../stress.h"

#define TEST_CONFIG_MIN_INTERVAL      0.05f
#define TEST_CONFIG_MAX_INTERVAL      10.0f
#define TEST_CONFIG_DEFAULT_INTERVAL  0.5f

#define TEST_CONFIG_MIN_THRESHOLD     0.5f
#define TEST_CONFIG_MAX_THRESHOLD     10.f
#define TEST_CONFIG_DEFAULT_THRESHOLD 2.0f

#define TEST_CONFIG_MIN_BREAK         1.0f
#define TEST_CONFIG_MAX_BREAK         60.0f
#define TEST_CONFIG_DEFAULT_BREAK     10.0f

#define TEST_CONFIG_DEFAULT_FILE      "nbfc.test-config.result.json"

const struct cli99_Option TestConfig_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-i|--input",       Option_TestConfig_Input,       cli99_RequiredArgument},
  {"-o|--output",      Option_TestConfig_Output,      cli99_RequiredArgument},
  {"-c|--cpu",         Option_TestConfig_Cpu,         cli99_RequiredArgument},
  {"-g|--gpu",         Option_TestConfig_Gpu,         cli99_RequiredArgument},
  {"-I|--interval",    Option_TestConfig_Interval,    cli99_RequiredArgument},
  {"-b|--break",       Option_TestConfig_Break,       cli99_RequiredArgument},
  {"-t|--threshold",   Option_TestConfig_Threshold,   cli99_RequiredArgument},
  {"command",          Option_TestConfig_Action,      cli99_NormalPositional},
  cli99_Options_End()
};

typedef enum NBFC_PACKED_ENUM {
  TestConfig_Action_None,
  TestConfig_Action_Run,
  TestConfig_Action_Evaluate,
} TestConfig_Action;

struct {
  TestConfig_Action action;
  const char* input;
  const char* output;
  size_t cpu_workers;
  size_t gpu_workers;
  float interval;
  float threshold;
  float break_;
} TestConfig_Options = {
  TestConfig_Action_None,
  NULL,
  NULL,
  0,
  0,
  TEST_CONFIG_DEFAULT_INTERVAL,
  TEST_CONFIG_DEFAULT_THRESHOLD,
  TEST_CONFIG_DEFAULT_BREAK,
};

TestConfig_Action TestConfig_Action_FromString(const char* s) {
  if (! strcmp(s, "run"))      return TestConfig_Action_Run;
  if (! strcmp(s, "evaluate")) return TestConfig_Action_Evaluate;
  return TestConfig_Action_None;
}

/*
 * Stores a Fan and its readings.
 *
 * fan:
 *   The fan object.
 *
 * readings:
 *   The readings of the fan in percent.
 */
struct FanWithReadings {
  Fan fan;
  array_of(float) readings;
};
typedef struct FanWithReadings FanWithReadings;
declare_array_of(FanWithReadings);

/*
 * Stores a group of sensor references and their readings.
 *
 * sensors:
 *   References to FS_TemperatureSource objects.
 *
 * readings:
 *   The sensor readings in celsius.
 */
struct SensorWithReadings {
  FS_TemperatureSource_References sensors;
  array_of(float) readings;
};
typedef struct SensorWithReadings SensorWithReadings;

/*
 * Stores a model configuration and their fan readings.
 *
 * name:
 *   The name of the configuration file.
 *
 * model_config:
 *   The parsed model configuration.
 *
 * fan_readings:
 *   An array of fans with their readings.
 *
 * score:
 *   The score of this config in percent (higher == better)
 *
 * inactive:
 *   Used for disabling a config.
 *   It is easier to mark a config as `inactive` than to pop it from the array.
 */
struct ConfigWithReadings {
  char* name;
  ModelConfig model_config;
  array_of(FanWithReadings) fan_readings;
  float score;
  bool inactive;
};
typedef struct ConfigWithReadings ConfigWithReadings;
typedef struct ConfigWithReadings* ConfigWithReadings_Ptr;
declare_array_of(ConfigWithReadings);
declare_array_of(ConfigWithReadings_Ptr);

/*
 * Stores the data that is needed for testing a list of configurations.
 *
 * cpu_readings:
 *   The readings for the CPU.
 *
 * gpu_readings:
 *   The readings for the GPU.
 *   Only active if `use_gpu` is true.
 *
 * configs_with_readings:
 *   An array of configurations and their readings.
 *
 * use_gpu:
 *   Specifies whether to use `gpu_readings`.
 */
struct ConfigTester {
  SensorWithReadings cpu_readings;
  SensorWithReadings gpu_readings;
  array_of(ConfigWithReadings) configs_with_readings;
  bool use_gpu;
};
typedef struct ConfigTester ConfigTester;

static void ConfigWithReadings_Free(ConfigWithReadings* cwr) {
  Mem_Free(cwr->name);
  ModelConfig_Free(&cwr->model_config);

  for_each_array(FanWithReadings*, fwr, cwr->fan_readings) {
    Mem_Free(fwr->readings.data);
  }
  Mem_Free(cwr->fan_readings.data);

  memset(cwr, 0, sizeof(ConfigWithReadings));
}

static Error ConfigWithReadings_Init(ConfigWithReadings* cwr, const char* file) {
  Error e;
  Trace trace;
  char path[PATH_MAX];

  memset(cwr, 0, sizeof(ConfigWithReadings));
  Trace_Init(&trace);

  // Set config name
  cwr->name = Mem_Strdup(file);

  // Load the configuration file
  e = ModelConfig_FindAndLoad(&cwr->model_config, path, file);
  if (e) {
    e = err_chain_string(e, path);
    goto end;
  }

  // Validate the configuration data (and silence warnings)
  Trace_Push(&trace, "%s", path);
  LogLevel old = Log_LogLevel;
  Log_LogLevel = LogLevel_Quiet;
  e = ModelConfig_Validate(&trace, &cwr->model_config);
  Log_LogLevel = old;
  if (e)
    goto end;

  // Create `fan_readings`
  const array_size_t fan_count = cwr->model_config.FanConfigurations.size;
  array_calloc(FanWithReadings, cwr->fan_readings, fan_count);

  for (array_size_t fan_idx = 0; fan_idx < fan_count; ++fan_idx) {
    Fan* fan = &cwr->fan_readings.data[fan_idx].fan;
    FanConfiguration* fan_config = &cwr->model_config.FanConfigurations.data[fan_idx];

    e = Fan_Init(fan, fan_config, &cwr->model_config);
    if (e) {
      e = err_chain_stringf(e, "%s: Fan #%zu", path, fan_idx);
      goto end;
    }

    cwr->fan_readings.size = fan_idx + 1;
  }

end:
  if (e)
    ConfigWithReadings_Free(cwr);

  return e;
}

static Error ConfigTester_Init(ConfigTester* tester, array_of(ConfigFile)* configs, bool use_gpu) {
  Error e;

  memset(tester, 0, sizeof(ConfigTester));

  tester->use_gpu = use_gpu;

  e = FS_TemperatureSources_AddTemperatureSources(&tester->cpu_readings.sensors, "@CPU");
  if (e)
    return e;

  if (use_gpu) {
    e = FS_TemperatureSources_AddTemperatureSources(&tester->gpu_readings.sensors, "@GPU");
    if (e)
      return e;
  }

  array_calloc(ConfigWithReadings, tester->configs_with_readings, configs->size);

  array_size_t size = 0;
  for_each_array(ConfigFile*, file, *configs) {
    ConfigWithReadings* cwr = &tester->configs_with_readings.data[size];
    e = ConfigWithReadings_Init(cwr, file->config_name);
    if (e) {
      Log_Warn("%s", err_print_all(e));
      continue;
    }

    ++size;
  }
  tester->configs_with_readings.size = size;

  return err_success();
}

static void ConfigTester_Free(ConfigTester* tester) {
  Mem_Free(tester->cpu_readings.sensors.data);
  Mem_Free(tester->cpu_readings.readings.data);

  Mem_Free(tester->gpu_readings.sensors.data);
  Mem_Free(tester->gpu_readings.readings.data);

  for_each_array(ConfigWithReadings*, cwr, tester->configs_with_readings) {
    ConfigWithReadings_Free(cwr);
  }

  Mem_Free(tester->configs_with_readings.data);

  memset(tester, 0, sizeof(ConfigTester));
}

static Error SensorWithReadings_TakeSample(SensorWithReadings* swr) {
  Error e;
  float temp;
  array_size_t idx;

  e = FS_TemperatureSources_GetTemperature(&swr->sensors, TemperatureAlgorithmType_Average, &temp);
  if (e)
    return e;

  idx = swr->readings.size;
  array_realloc(float, swr->readings, (idx + 1));
  swr->readings.data[idx] = temp;
  swr->readings.size = idx + 1;
  return err_success();
}

static Error ConfigWithReadings_TakeSample(ConfigWithReadings* cwr) {
  Error e;
  float speed;
  const LogLevel old_log_level = Log_LogLevel;

  for_each_array(FanWithReadings*, fwr, cwr->fan_readings) {
    // Read fan speed from EC (suppress warnings)
    Log_LogLevel = LogLevel_Quiet;
    e = Fan_UpdateCurrentSpeed(&fwr->fan);
    Log_LogLevel = old_log_level;

    if (e)
      return err_chain_stringf(e, "%s", fwr->fan.fanConfig->FanDisplayName);

    speed = Fan_GetCurrentSpeed(&fwr->fan);
    const array_size_t idx = fwr->readings.size;
    array_realloc(float, fwr->readings, idx + 1);
    fwr->readings.data[idx] = speed;
    fwr->readings.size = idx + 1;
  }

  return err_success();
}

static Error ConfigTester_TakeFanSamples(ConfigTester* tester) {
  Error e;

  for_each_array(ConfigWithReadings*, cwr, tester->configs_with_readings) {
    if (cwr->inactive)
      continue;

    e = ConfigWithReadings_TakeSample(cwr);
    if (e) {
      Log_Warn("%s: %s", cwr->name, err_print_all(e));
      cwr->inactive = true;
    }
  }

  return err_success();
}

static Error ConfigTester_TakeSamples(ConfigTester* tester) {
  Error e;

  e = SensorWithReadings_TakeSample(&tester->cpu_readings);
  if (e)
    return e;

  if (tester->use_gpu) {
    e = SensorWithReadings_TakeSample(&tester->gpu_readings);
    if (e)
      return e;
  }

  // Clear the Embedded Controller cache before iterating over all
  // configurations in ConfigTester_TakeFanSamples().
  EC_Cache_Clear();
  return ConfigTester_TakeFanSamples(tester);
}

static array_of(double) ArrayOfFloat_To_ArrayOfDouble(const array_of(float)* arr) {
  array_of(double) ret = {0};
  array_calloc(double, ret, arr->size);
  array_size_t size = 0;
  for_each_array(float*, f, *arr)
    ret.data[size++] = *f;
  ret.size = size;
  return ret;
}

/*
 * Calculates how much two arrays differ.
 *
 * Returns a score in percent (higher == less different).
 */
static float Array_CalculateDiffScore(const array_of(float)* a, const array_of(float)* b) {
  array_of(double) ad = ArrayOfFloat_To_ArrayOfDouble(a);
  array_of(double) bd = ArrayOfFloat_To_ArrayOfDouble(b);

  float score = spearman_correlation(ad.data, bd.data, ad.size) * 100.0f;

  Mem_Free(ad.data);
  Mem_Free(bd.data);

  return score;
}

static Error ConfigWithReadings_CalculateScore(ConfigWithReadings* cwr, const ConfigTester* tester) {
  float score;
  float best_score = 0.0f;

  // Defensive: We need at least one CPU reading
  if (tester->cpu_readings.readings.size == 0)
    return err_string("CPU readings length is zero");

  // Defensive: We need at least one GPU reading
  if (tester->use_gpu && tester->gpu_readings.readings.size == 0)
    return err_string("GPU readings length is zero");

  // Defensive: CPU readings and GPU readings need to have the same size
  if (tester->use_gpu &&
      tester->gpu_readings.readings.size != tester->cpu_readings.readings.size)
    return err_string("CPU readings length != GPU readings length");

  // Defensive: Fan readings need to have the same size as CPU/GPU readings
  for_each_array(FanWithReadings*, fwr, cwr->fan_readings)
    if (fwr->readings.size != tester->cpu_readings.readings.size)
      return err_stringf("%s: Fan readings length != CPU readings length", cwr->name);

  // Special case: We only have one fan
  if (cwr->fan_readings.size == 1) {
    score = Array_CalculateDiffScore(&cwr->fan_readings.data[0].readings, &tester->cpu_readings.readings);
    if (score > best_score)
      best_score = score;

    if (! tester->use_gpu)
      goto end;

    score = Array_CalculateDiffScore(&cwr->fan_readings.data[0].readings, &tester->gpu_readings.readings);
    if (score > best_score)
      best_score = score;

    goto end;
  }

  // We have GPU readings and more than one fan.
  if (tester->use_gpu) {
    for_enumerate_array(array_size_t, i, cwr->fan_readings) {
      for_enumerate_array(array_size_t, j, cwr->fan_readings) {
        if (i == j)
          continue;

        const FanWithReadings* i_fwr = &cwr->fan_readings.data[i];
        const FanWithReadings* j_fwr = &cwr->fan_readings.data[j];

        score =
          Array_CalculateDiffScore(&i_fwr->readings, &tester->cpu_readings.readings) +
          Array_CalculateDiffScore(&j_fwr->readings, &tester->gpu_readings.readings);

        if (score > best_score)
          best_score = score;
      }
    }

    goto end;
  }

  // We don't have GPU readings
  for_each_array(FanWithReadings*, fwr, cwr->fan_readings) {
    score = Array_CalculateDiffScore(&fwr->readings, &tester->cpu_readings.readings);

    if (score > best_score)
      best_score = score;
  }

end:
  cwr->score = best_score;
  return err_success();
}

static bool TestConfig_ReadingsDidChange(array_of(float)* array, array_size_t num, float threshold) {
  if (array->size < num)
    return true;

  const float last_temp = array->data[array->size - 1];

  for (array_size_t i = 2; i <= num; ++i) {
    if (fabs(last_temp - array->data[array->size - i]) >= threshold)
      return true;
  }

  return false;
}

static void ConfigTester_PrintProgress(const ConfigTester* tester) {
  if (tester->cpu_readings.readings.size == 0)
    return;

  if (tester->use_gpu)
    Log_Info("@CPU: %.2f  @GPU: %.2f",
        tester->cpu_readings.readings.data[tester->cpu_readings.readings.size - 1],
        tester->gpu_readings.readings.data[tester->gpu_readings.readings.size - 1]);
  else
    Log_Info("@CPU: %.2f",
        tester->cpu_readings.readings.data[tester->cpu_readings.readings.size - 1]);
}

static Error ConfigTester_StressCpuAndTakeSamples(ConfigTester* tester) {
  Error e;

  size_t num_workers = TestConfig_Options.cpu_workers;
  if (! num_workers)
    num_workers = Stress_GetNumProcessors();
  if (! num_workers) {
    Log_Warn("Could not get number of processors. Falling back to 8");
    num_workers = 8;
  }

  e = Stress_CPU_Start(num_workers);
  if (e)
    goto end;

  while (true) {
    sleep_ms((unsigned int) (1000.0f * TestConfig_Options.interval));
    e = ConfigTester_TakeSamples(tester);
    if (e)
      goto end;

    ConfigTester_PrintProgress(tester);

    if (! TestConfig_ReadingsDidChange(
            &tester->cpu_readings.readings,
            (array_size_t) ceilf(TestConfig_Options.break_ / TestConfig_Options.interval),
            TestConfig_Options.threshold))
      break;
  }

end:
  Stress_CPU_End();
  return e;
}

static Error ConfigTester_StressGpuAndTakeSamples(ConfigTester* tester) {
  Error e;

  e = Stress_GPU_Start(TestConfig_Options.gpu_workers);
  if (e)
    goto end;

  while (true) {
    sleep_ms((unsigned int) (1000.0f * TestConfig_Options.interval));
    e = ConfigTester_TakeSamples(tester);
    if (e)
      goto end;

    ConfigTester_PrintProgress(tester);

    if (! TestConfig_ReadingsDidChange(
            &tester->gpu_readings.readings,
            (array_size_t) ceilf(TestConfig_Options.break_ / TestConfig_Options.interval),
            TestConfig_Options.threshold))
      break;
  }

end:
  Stress_GPU_End();
  return e;
}

static void TestConfig_SortResultByScore(array_of(ConfigWithReadings_Ptr)* result) {
  if (! result->size)
    return;

  /* Bubble sort - ascending */
  for (array_size_t i = 0; i < result->size - 1; ++i) {
    for (array_size_t j = 0; j < result->size - i - 1; ++j) {
      ConfigWithReadings_Ptr* a = &result->data[j];
      ConfigWithReadings_Ptr* b = &result->data[j + 1];
      if ((*a)->score > (*b)->score) {
        const ConfigWithReadings_Ptr swap = *a;
        *a = *b;
        *b = swap;
      }
    }
  }
}

static void ConfigTester_PrintResults(const ConfigTester* tester) {
  array_of(ConfigWithReadings_Ptr) result = {0};
  array_calloc(ConfigWithReadings_Ptr, result, tester->configs_with_readings.size);
  array_size_t size = 0;
  for_each_array(ConfigWithReadings*, cwr, tester->configs_with_readings) {
    if (! cwr->inactive)
      result.data[size++] = cwr;
  }
  result.size = size;

  TestConfig_SortResultByScore(&result);

  for_each_array(ConfigWithReadings_Ptr*, cwr, result) {
    printf("%s (%.2f%%)\n", (*cwr)->name, (*cwr)->score);
  }

  Mem_Free(result.data);
}

static nx_json* ArrayOfFloat_ToJson(const array_of(float)* array, nx_json* parent, const char* key) {
  nx_json* arr = create_json_array(key, parent);

  for_each_array(float*, f, *array) {
    create_json_double(NULL, arr, *f);
  }

  return arr;
}

static nx_json* ConfigWithReadings_ToJson(const ConfigWithReadings* cwr, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  create_json_string("name", obj, cwr->name);

  nx_json* arr = create_json_array("fans", obj);
  for_each_array(FanWithReadings*, fwr, cwr->fan_readings) {
    ArrayOfFloat_ToJson(&fwr->readings, arr, NULL);
  }

  return obj;
}

static nx_json* ConfigTester_ToJson(const ConfigTester* tester, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  create_json_bool("use_gpu", obj, tester->use_gpu);

  ArrayOfFloat_ToJson(&tester->cpu_readings.readings, obj, "cpu");
  ArrayOfFloat_ToJson(&tester->gpu_readings.readings, obj, "gpu");

  nx_json* arr = create_json_array("configs", obj);
  for_each_array(ConfigWithReadings*, cwr, tester->configs_with_readings) {
    if (cwr->inactive)
      continue;

    ConfigWithReadings_ToJson(cwr, arr, NULL);
  }

  return obj;
}

static Error ArrayOfFloat_FromJson(array_of(float)* out, const nx_json* json) {
  out->size = 0;
  out->data = NULL;

  if (json->type != NX_JSON_ARRAY)
    return err_string("Not an array");

  array_calloc(float, *out, json->val.children.length);

  nx_json_for_each(item, json) {
    float val;

    if (item->type == NX_JSON_DOUBLE)
      val = (float) item->val.dbl;
    else if (item->type == NX_JSON_INTEGER)
      val = (float) item->val.i;
    else
      return err_string("Array contains non-floats");

    out->data[out->size++] = val;
  }

  return err_success();
}

static Error ArrayOfFanWithReadings_FromJson(array_of(FanWithReadings)* out, const nx_json* json) {
  Error e = err_success();
  out->size = 0;
  out->data = NULL;

  if (json->type != NX_JSON_ARRAY)
    return err_string("Not an array");

  array_calloc(FanWithReadings, *out, json->val.children.length);

  nx_json_for_each(item, json) {
    e = ArrayOfFloat_FromJson(&out->data[out->size].readings, item);
    if (e)
      return e;

    out->size++;
  }

  return err_success();
}

static Error ConfigWithReadings_FromJson(ConfigWithReadings* cwr, const nx_json* json) {
  Error e = err_success();
  memset(cwr, 0, sizeof(ConfigWithReadings));

  if (json->type != NX_JSON_OBJECT)
    return err_string("Not an object");

  nx_json_for_each(item, json) {
    if (! strcmp(item->key, "name")) {
      if (item->type == NX_JSON_STRING)
        cwr->name = Mem_Strdup(item->val.text);
      else
        e = err_string("Not a string");
    }
    else if (! strcmp(item->key, "fans")) {
      e = ArrayOfFanWithReadings_FromJson(&cwr->fan_readings, item);
    }
    else {
      e = err_string("Unknown key");
    }

    if (e)
      return err_chain_string(e, item->key);
  }

  if (! cwr->name)
    return err_string("Missing \"name\" field");

  return err_success();
}

static Error ArrayOfConfigWithReadings_FromJson(array_of(ConfigWithReadings)* out, const nx_json* json) {
  Error e = err_success();
  out->size = 0;
  out->data = NULL;

  if (json->type != NX_JSON_ARRAY)
    return err_string("Not an array");

  array_calloc(ConfigWithReadings, *out, json->val.children.length);

  nx_json_for_each(item, json) {
    e = ConfigWithReadings_FromJson(&out->data[out->size], item);
    if (e)
      return e;
    out->size++;
  }

  return err_success();
}

static Error ConfigTester_FromJson(ConfigTester* tester, const nx_json* json) {
  Error e = err_success();
  memset(tester, 0, sizeof(ConfigTester));

  if (json->type != NX_JSON_OBJECT)
    return err_string("Not an object");

  nx_json_for_each(item, json) {
    if (! strcmp(item->key, "configs"))
      e = ArrayOfConfigWithReadings_FromJson(&tester->configs_with_readings, item);
    else if (! strcmp(item->key, "cpu"))
      e = ArrayOfFloat_FromJson(&tester->cpu_readings.readings, item);
    else if (! strcmp(item->key, "gpu"))
      e = ArrayOfFloat_FromJson(&tester->gpu_readings.readings, item);
    else if (! strcmp(item->key, "use_gpu")) {
      if (item->type == NX_JSON_BOOL)
        tester->use_gpu = item->val.u;
      else
        e = err_string("Not a bool");
    }
    else
      e = err_string("Unknown key");

    if (e)
      return err_chain_string(e, item->key);
  }

  return err_success();
}

static int TestConfig_Run(void) {
  Error e;
  const char* infile;
  const char* outfile;
  array_of(ConfigFile) files = {0};
  ConfigTester tester = {0};

  check_root();

  // Use STDIN as input file if no file is given
  infile = TestConfig_Options.input;
  if (!infile || !strcmp(infile, "-"))
    infile = "/dev/stdin";

  // Use default output file if no file is given
  outfile = TestConfig_Options.output;
  if (! outfile)
    outfile = TEST_CONFIG_DEFAULT_FILE;
  else if (! strcmp(outfile, "-"))
    outfile = "/dev/stdout";

  // Check if stress is installed
  e = Stress_IsInstalled();
  if (e)
    goto error;

  // Read configuration filenames from input file
  e = ConfigFiles_FromFile(&files, infile);
  if (e) {
    e = err_chain_string(e, infile);
    goto error;
  }

  // Initialize Embedded Controller
  e = EC_FindWorking(&ec);
  if (e)
    goto error;

  e = ec->Open();
  if (e)
    goto error;

  // Cache Embedded Controller readings
  EC_Cache_Controller = ec;
  ec = &EC_Cache_VTable;

  // Initialize ACPI-Call
  e = AcpiCall_Open();
  if (e)
    goto error;

  // Initialize Sensors
  e = FS_Sensors_Init();
  if (e)
    goto error;

  // Initialize ConfigTester
  e = ConfigTester_Init(&tester, &files, (TestConfig_Options.gpu_workers > 0));
  if (e)
    goto error;

  // Take first sample
  e = ConfigTester_TakeSamples(&tester);
  if (e)
    goto error;

  // Start stressing system while taking samples
  e = ConfigTester_StressCpuAndTakeSamples(&tester);
  if (e)
    goto error;

  if (tester.use_gpu) {
    e = ConfigTester_StressGpuAndTakeSamples(&tester);
    if (e)
      goto error;
  }

  // Write result file
  nx_json root = {0};
  nx_json* js = ConfigTester_ToJson(&tester, &root, NULL);
  int fd = open(outfile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  if (fd < 0) {
    e = err_stdlib(outfile);
    goto error;
  }

  if (! nxjson_write_to_fd(js, fd, 2))
    e = err_stdlib(outfile);

#if STRICT_CLEANUP
  nx_json_free(js);
  close(fd);
#endif

error:
  if (e)
    Log_Error("%s", err_print_all(e));

#if STRICT_CLEANUP
  ConfigFiles_Free(&files);
  ConfigTester_Free(&tester);
  if (ec)
    ec->Close();
#endif

  return e ? NBFC_EXIT_FAILURE : NBFC_EXIT_SUCCESS;
}

static int TestConfig_Evaluate(void) {
  Error e;
  FileResult res;
  char* content = NULL;
  const nx_json* json = NULL;
  ConfigTester tester = {0};

  // ==========================================================================
  // Read input file
  // ==========================================================================

  const char* infile = TestConfig_Options.input;
  if (! infile)
    infile = TEST_CONFIG_DEFAULT_FILE;
  else if (! strcmp(infile, "-"))
    infile = "/dev/stdin";

  res = File_ReadDynamic(&content, infile);
  if (! res.ok) {
    e = err_stdlib(infile);
    goto error;
  }

  json = nx_json_parse_utf8(content);
  if (! json) {
    e = err_nxjson(infile);
    goto error;
  }

  e = ConfigTester_FromJson(&tester, json);
  if (e) {
    e = err_chain_string(e, infile);
    goto error;
  }

  // ==========================================================================
  // Calculate the score
  // ==========================================================================

  for_each_array(ConfigWithReadings*, cwr, tester.configs_with_readings) {
    e = ConfigWithReadings_CalculateScore(cwr, &tester);
    if (e)
      goto error;
  }

  // ==========================================================================
  // Print the results
  // ==========================================================================

  ConfigTester_PrintResults(&tester);

error:
  if (e) {
    Log_Error("%s", err_print_all(e));
  }

#if STRICT_CLEANUP
  Mem_Free(content);
  nx_json_free(json);
  ConfigTester_Free(&tester);
#endif

  return e ? NBFC_EXIT_FAILURE : NBFC_EXIT_SUCCESS;
}

int TestConfig(void) {
  switch (TestConfig_Options.action) {
  case TestConfig_Action_None:
    Log_Error("test-config: Missing command");
    return NBFC_EXIT_CMDLINE;
  case TestConfig_Action_Run:
    return TestConfig_Run();
  case TestConfig_Action_Evaluate:
    return TestConfig_Evaluate();
  }

  return NBFC_EXIT_FAILURE;
}
