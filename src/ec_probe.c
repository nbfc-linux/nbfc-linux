#define _XOPEN_SOURCE  500 // unistd.h: export pwrite()/pread()
#define _DEFAULT_SOURCE    // endian.h:

#include "nbfc.h"
#include "macros.h"
#include "sleep.h"
#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "acpi_call.h"
#include "model_config.h"
#include "optparse/optparse.h"
#include "parse_number.h"
#include "parse_unumber.h"
#include "parse_double.h"
#include "help/ec_probe.help.h"
#include "program_name.c"
#include "log.h"

#include <float.h>   // FLT_MAX
#include <stdbool.h> // bool
#include <stdio.h>   // printf, fprintf, fopen, fread, fclose
#include <stdint.h>  // uint8_t, uint16_t
#include <stdlib.h>  // strtoll
#include <string.h>  // strcmp
#include <limits.h>  // INT_MAX
#include <locale.h>  // setlocale, LC_NUMERIC
#include <signal.h>  // signal, SIGINT, SIGTERM
#include <unistd.h>  // geteuid, STDOUT_FILENO

#include "error.c"             // src
#include "ec.c"                // src

#if ENABLE_EC_DEV_PORT
#include "ec_linux.c"          // src
#endif

#if ENABLE_EC_SYS || ENABLE_EC_ACPI
#include "ec_sys_linux.c"      // src
#endif

#include "acpi_call.c"         // src
#include "log.c"               // src
#include "optparse/optparse.c" // src
#include "memory.c"            // src
#include "nxjson.c"            // src
#include "model_config.c"      // src
#include "stack_memory.c"      // src
#include "trace.c"             // src
#include "file_utils.c"        // src

#define Console_Black       "\033[0;30m"
#define Console_Red         "\033[0;31m"
#define Console_Green       "\033[0;32m"
#define Console_Yelllow     "\033[0;33m"
#define Console_Blue        "\033[0;34m"
#define Console_Magenta     "\033[0;35m"
#define Console_Cyan        "\033[0;36m"
#define Console_White       "\033[0;37m"
#define Console_Gray        "\033[0;38m"

#define Console_BoldBlack   "\033[1;30m"
#define Console_BoldRed     "\033[1;31m"
#define Console_BoldGreen   "\033[1;32m"
#define Console_BoldYelllow "\033[1;33m"
#define Console_BoldBlue    "\033[1;34m"
#define Console_BoldMagenta "\033[1;35m"
#define Console_BoldCyan    "\033[1;36m"
#define Console_BoldWhite   "\033[1;37m"
#define Console_BoldGray    "\033[1;38m"

#define Console_Reset       "\033[0;0m"
#define Console_Clear       "\033[1;1H\033[2J"

#define             RegistersSize 256
typedef uint8_t     RegisterBuf[RegistersSize];
typedef const char* RegisterColors[RegistersSize];
static RegisterBuf  Registers_Log[32768];

static void         Register_PrintRegister(RegisterBuf*, RegisterColors);
static inline void  Register_FromEC(RegisterBuf*);
static inline void  Register_ToEC(RegisterBuf*);
static void         Register_PrintWatch(RegisterBuf*, RegisterBuf*, RegisterBuf*);
static void         Register_PrintMonitor(RegisterBuf*, int);
static void         Register_WriteMonitorReport(RegisterBuf*, int, FILE*);
static void         Register_PrintDump(RegisterBuf*, bool);
static int          Register_LoadDump(RegisterBuf*, FILE*);
static void         Handle_Signal(int);

static EC_VTable*   ec;
static volatile int quit;

static int Read();
static int Write();
static int Dump();
static int Load();
static int Monitor();
static int Watch();
static int AcpiCall();
static int Shell();

enum Command {
  Command_Read,
  Command_Write,
  Command_Dump,
  Command_Load,
  Command_Monitor,
  Command_Watch,
  Command_AcpiCall,
  Command_Shell,
  Command_Help,
  Command_End
};

static enum Command Command_FromString(const char* s) {
  const char* cmds[] = { "read", "write", "dump", "load", "monitor", "watch", "acpi_call", "shell", "help" };

  for (int i = 0; i < ARRAY_SSIZE(cmds); ++i)
    if (!strcmp(cmds[i], s))
      return (enum Command) i;

  return Command_End;
}

static const char* HelpTexts[] = {
  EC_PROBE_READ_HELP_TEXT,
  EC_PROBE_WRITE_HELP_TEXT,
  EC_PROBE_DUMP_HELP_TEXT,
  EC_PROBE_LOAD_HELP_TEXT,
  EC_PROBE_MONITOR_HELP_TEXT,
  EC_PROBE_WATCH_HELP_TEXT,
  EC_PROBE_ACPI_CALL_HELP_TEXT,
  EC_PROBE_SHELL_HELP_TEXT,
  EC_PROBE_HELP_TEXT,
};

enum Option {
  Option_None,
  Option_Help,
  Option_Version,
  Option_EmbeddedController,
  Option_Command,
  Option_Word,
  Option_Register,
  Option_Value,
  Option_Color,
  Option_NoColor,
  Option_File,
  Option_Report,
  Option_Clearly,
  Option_Decimal,
  Option_Timespan,
  Option_Interval,
  Option_AcpiCallMethod,
  Option_AcpiCallArgument,
};

static const cli99_option main_options[] = {
  {"-e|--embedded-controller", Option_EmbeddedController,  1},
  {"-h|--help",                Option_Help,                0},
  {"--version",                Option_Version,             0},
  {"command",                  Option_Command,             1|cli99_required_option},
  cli99_options_end()
};

static const cli99_option read_command_options[] = {
  cli99_include_options(&main_options),
  {"-w|--word",                Option_Word,                0},
  {"register",                 Option_Register,            1|cli99_required_option},
  cli99_options_end()
};

static const cli99_option write_command_options[] = {
  cli99_include_options(&main_options),
  {"-w|--word",                Option_Word,                0},
  {"register",                 Option_Register,            1|cli99_required_option},
  {"value",                    Option_Value,               1|cli99_required_option},
  cli99_options_end()
};

static const cli99_option dump_command_options[] = {
  cli99_include_options(&main_options),
  {"-c|--color",               Option_Color,               0},
  {"-C|--no-color",            Option_NoColor,             0},
  cli99_options_end()
};

static const cli99_option load_command_options[] = {
  cli99_include_options(&main_options),
  {"file",                     Option_File,                1|cli99_required_option},
  cli99_options_end()
};

static const cli99_option monitor_command_options[] = {
  cli99_include_options(&main_options),
  {"-r|--report",              Option_Report,              1},
  {"-c|--clearly",             Option_Clearly,             0},
  {"-d|--decimal",             Option_Decimal,             0},
  {"-t|--timespan",            Option_Timespan,            1},
  {"-i|--interval",            Option_Interval,            1},
  cli99_options_end()
};

static const cli99_option watch_command_options[] = {
  cli99_include_options(&main_options),
  {"-t|--timespan",            Option_Timespan,            1},
  {"-i|--interval",            Option_Interval,            1},
  cli99_options_end()
};

static const cli99_option acpi_call_command_options[] = {
  cli99_include_options(&main_options),
  {"method",                   Option_AcpiCallMethod,      1|cli99_required_option},
  {"arg1",                     Option_AcpiCallArgument,    1},
  {"arg2",                     Option_AcpiCallArgument,    1},
  {"arg3",                     Option_AcpiCallArgument,    1},
  {"arg4",                     Option_AcpiCallArgument,    1},
  {"arg5",                     Option_AcpiCallArgument,    1},
  {"arg6",                     Option_AcpiCallArgument,    1},
  {"arg7",                     Option_AcpiCallArgument,    1},
  {"arg8",                     Option_AcpiCallArgument,    1},
  cli99_options_end()
};

static const cli99_option* Options[] = {
  read_command_options,
  write_command_options,
  dump_command_options,
  load_command_options,
  monitor_command_options,
  watch_command_options,
  acpi_call_command_options,
  main_options, // shell
  main_options, // help
};

enum UseColor {
  ColorAuto,
  ColorEnable,
  ColorDisable,
};

static struct {
  int           timespan;
  float         interval;
  const char*   report;
  const char*   file;
  bool          clearly;
  bool          decimal;
  uint8_t       register_;
  uint16_t      value;
  bool          use_word;
  enum UseColor use_color;
  const char*   acpi_call_method;
  uint64_t      acpi_call_args[8];
  int           acpi_call_args_size;
} options = {0};

const char RegisterHeader[] =
  "---|------------------------------------------------\n"
  "   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
  "---|------------------------------------------------\n";

int main(int argc, char* const argv[]) {
  if (argc == 1) {
    printf(EC_PROBE_HELP_TEXT, argv[0]);
    return NBFC_EXIT_CMDLINE;
  }

  Program_Name_Set(argv[0]);
  setlocale(LC_NUMERIC, "C"); // for parsing floats

  options.interval = 0.5;
  ec = NULL;
  enum Command cmd = Command_Help;

  cli99 p;
  cli99_Init(&p, argc, argv, main_options, cli99_options_python);

  int o;
  const char* err;
  while ((o = cli99_GetOpt(&p))) {
    switch (o) {
    case Option_Command:
      cmd = Command_FromString(p.optarg);

      if (cmd == Command_End) {
        Log_Error("Invalid command: %s\n", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      if (cmd == Command_Help) {
        printf(EC_PROBE_HELP_TEXT, argv[0]);
        return 0;
      }

      cli99_SetOptions(&p, Options[cmd], false);
      break;
    case Option_Register:
      options.register_ = parse_number(p.optarg, 0, 255, &err);
      if (err) {
        Log_Error("Register: %s: %s\n", p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Value:
      options.value = parse_number(p.optarg, 0, 65535, &err);
      if (err) {
        Log_Error("Value: %s: %s\n", p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Help:     printf(HelpTexts[cmd], argv[0]);         return 0;
    case Option_Version:  printf("ec_probe " NBFC_VERSION "\n");   return 0;
    case Option_Clearly:  options.clearly  = 1;                    break;
    case Option_Decimal:  options.decimal  = 1;                    break;
    case Option_Word:     options.use_word = 1;                    break;
    case Option_Report:   options.report   = p.optarg;             break;
    case Option_Color:    options.use_color = ColorEnable;         break;
    case Option_NoColor:  options.use_color = ColorDisable;        break;
    case Option_File:     options.file = p.optarg;                 break;
    case Option_EmbeddedController:
      switch(EmbeddedControllerType_FromString(p.optarg)) {
#if ENABLE_EC_SYS
        case EmbeddedControllerType_ECSysLinux:     ec = &EC_SysLinux_VTable;      break;
#endif
#if ENABLE_EC_ACPI
        case EmbeddedControllerType_ECSysLinuxACPI: ec = &EC_SysLinux_ACPI_VTable; break;
#endif
#if ENABLE_EC_DEV_PORT
        case EmbeddedControllerType_ECLinux:        ec = &EC_Linux_VTable;         break;
#endif
        default:
          Log_Error("-e|--embedded-controller: Invalid value: %s\n", p.optarg);
          return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Timespan:
      options.timespan = parse_number(p.optarg, 1, INT_MAX, &err);
      if (err) {
        Log_Error("-t|--timespan: %s: %s\n", p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Interval:
      options.interval = parse_double(p.optarg, 0.1, FLT_MAX, &err);
      if (err) {
        Log_Error("-i|--interval: %s: %s\n", p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_AcpiCallMethod:
      options.acpi_call_method = p.optarg;
      break;
    case Option_AcpiCallArgument:
      options.acpi_call_args[options.acpi_call_args_size++] = parse_unumber(p.optarg, 0, UINT64_MAX, &err);
      if (err) {
        Log_Error("Argument: %s: %s\n", p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    default:
      cli99_ExplainError(&p);
      return NBFC_EXIT_CMDLINE;
    }
  }

  if (! cli99_End(&p)) {
    Log_Error("Too much arguments\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (! cli99_CheckRequired(&p)) {
    cli99_ExplainError(&p);
    return NBFC_EXIT_CMDLINE;
  }

  if (geteuid()) {
    Log_Error("This program must be run as root\n");
    return NBFC_EXIT_FAILURE;
  }

  signal(SIGINT,  Handle_Signal);
  signal(SIGTERM, Handle_Signal);

  if (ec == NULL) {
    Error* e = EC_FindWorking(&ec);
    e_die();
  }

  Error* e = ec->Open();
  e_die();

  switch (cmd) {
  case Command_Dump:     return Dump();
  case Command_Load:     return Load();
  case Command_Read:     return Read();
  case Command_Write:    return Write();
  case Command_Monitor:  return Monitor();
  case Command_Watch:    return Watch();
  case Command_AcpiCall: return AcpiCall();
  case Command_Shell:    return Shell();
  default:               return NBFC_EXIT_FAILURE;
  }
}

static int Read() {
  if (options.use_word) {
    uint16_t word;
    Error* e = ec->ReadWord(options.register_, &word);
    e_die();
    printf("%d (0x%.2X)\n", word, word);
  }
  else {
    uint8_t byte;
    Error* e = ec->ReadByte(options.register_, &byte);
    e_die();
    printf("%d (0x%.2X)\n", byte, byte);
  }

  return 0;
}

static int Write() {
  if (options.use_word) {
    Error* e = ec->WriteWord(options.register_, options.value);
    e_die();
  }
  else {
    if (options.value > 255) {
      Log_Error("write: Value too big: %d\n", options.value);
      return NBFC_EXIT_CMDLINE;
    }
    Error* e = ec->WriteByte(options.register_, options.value);
    e_die();
  }

  return 0;
}

static int Dump() {
  bool use_color = false;
  RegisterBuf register_buf;

  Register_FromEC(&register_buf);

  switch (options.use_color) {
  case ColorAuto:     use_color = isatty(STDOUT_FILENO); break;
  case ColorEnable:   use_color = true;                  break;
  case ColorDisable:  use_color = false;                 break;
  }

  Register_PrintDump(&register_buf, use_color);
  return 0;
}

static int Load() {
  FILE* infile;

  if (! strcmp(options.file, "-"))
    infile = stdin;
  else {
    infile = fopen(options.file, "r");
    if (! infile) {
      Log_Error("Error opening file: %s: %s\n", options.file, strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
  }

  RegisterBuf register_buf;
  int ret = Register_LoadDump(&register_buf, infile);
  fclose(infile);

  if (ret == 0) {
    Register_ToEC(&register_buf);
  }

  return ret;
}

static int Monitor() {
  int max_loops = INT_MAX;

  if (options.timespan)
    max_loops = options.timespan / options.interval;

  RegisterBuf* regs = Registers_Log;
  int size = ARRAY_SSIZE(Registers_Log);
  int loops;
  for (loops = 0; !quit && loops < max_loops && --size; ++loops) {
    Register_FromEC(regs + loops);
    Register_PrintMonitor(regs, loops);
    sleep_ms(options.interval * 1000);
  }

  if (options.report) {
    FILE* fh = fopen(options.report, "w");
    if (! fh) {
      Log_Error("%s: %s\n", options.report, strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
    Register_WriteMonitorReport(regs, loops, fh);
    fclose(fh);
  }

  return 0;
}

static int Watch() {
  int max_loops = INT_MAX;

  if (options.timespan)
    max_loops = options.timespan / options.interval;

  int size = ARRAY_SSIZE(Registers_Log);
  RegisterBuf* regs = Registers_Log;
  Register_FromEC(regs);
  for (int loops = 1; !quit && loops < max_loops && --size; ++loops) {
    Register_FromEC(regs + loops);
    Register_PrintWatch(regs , regs + loops, regs + loops - 1);
    sleep_ms(options.interval * 1000);
  }

  return 0;
}

static int AcpiCall() {
  Error* e;
  char cmd[1024];

  e = AcpiCall_Open();
  e_die();

  char fmt[] = "%s 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX";
  fmt[2 + options.acpi_call_args_size * 6] = '\0';

  ssize_t cmd_len = snprintf(cmd, sizeof(cmd), fmt,
    options.acpi_call_method,
    options.acpi_call_args[0],
    options.acpi_call_args[1],
    options.acpi_call_args[2],
    options.acpi_call_args[3],
    options.acpi_call_args[4],
    options.acpi_call_args[5],
    options.acpi_call_args[6],
    options.acpi_call_args[7]
  );

  if (cmd_len == -1 || cmd_len >= (sizeof(cmd))) {
    Log_Error("Method (including arguments) is too long\n");
    return NBFC_EXIT_FAILURE;
  }

  uint64_t out;
  e = AcpiCall_Call(cmd, cmd_len, &out);
  e_die();
  printf("0x%lX\n", out);

  return NBFC_EXIT_SUCCESS;
}

static void Handle_Signal(int sig) {
  quit = sig;
}

// ============================================================================
// Registers code
// ============================================================================

static void Register_PrintRegister(RegisterBuf* self, RegisterColors color) {
  if (color)
    printf(Console_Reset);

  printf("%s", RegisterHeader);

  for (int i = 0; i <= 0xF0; i += 0x10) {
    if (color)
      printf(Console_Reset);

    printf("%.2X |", i);

    if (color) {
      for (int j = 0; j <= 0xF; ++j)
        printf("%s %.2X", color[i + j], my[i + j]);
    }
    else {
      for (int j = 0; j <= 0xF; ++j)
        printf(" %.2X", my[i + j]);
    }

    printf("\n");
  }
}

static inline void Register_FromEC(RegisterBuf* self) {
  for (int i = 0; i < RegistersSize; i++)
    ec->ReadByte(i, &my[i]);
}

static inline void Register_ToEC(RegisterBuf* self) {
  for (int i = 0; i < RegistersSize; ++i)
    ec->WriteByte(i, my[i]);
}

static void Register_PrintWatch(RegisterBuf* all_readings, RegisterBuf* current, RegisterBuf* previous) {
  RegisterColors colors;

  for (int register_ = 0; register_ < RegistersSize; ++register_) {
    const uint8_t byte = (*current)[register_];
    const uint8_t diff = byte - (*previous)[register_];
    bool has_changed = false;

    uint8_t save = byte;
    for (range(RegisterBuf*, r, all_readings, previous)) {
      if (save != (*r)[register_]) {
        has_changed = true;
        break;
      }
    }

    /**/ if (diff)          colors[register_] = Console_Yelllow;
    else if (has_changed)   colors[register_] = Console_BoldBlue;
    else if (byte == 0xFF)  colors[register_] = Console_White;
    else if (byte)          colors[register_] = Console_BoldWhite;
    else                    colors[register_] = Console_BoldBlack;
  }

  Register_PrintRegister(current, colors);
}

static void Register_PrintMonitor(RegisterBuf* readings, int size) {
  printf(Console_Clear);

  for (int register_ = 0; register_ < RegistersSize; ++register_) {
    bool register_has_changed = false;
    for (range(int, i, 0, size)) {
      if (readings[0][register_] != readings[i][register_]) {
        register_has_changed = true;
        break;
      }
    }

    if (! register_has_changed)
      continue;

    printf(Console_Green "0x%.2X:", register_);
    uint8_t byte = readings[0][register_];
    for (range(int, i, max(size - 24, 0), size)) {
      const uint8_t diff = byte - readings[i][register_];
      byte = readings[i][register_];
      if (diff)
        printf(Console_BoldBlue " %.2X", byte);
      else
        printf(Console_BoldWhite " %.2X", byte);
    }
    printf("\n");
  }
}

static void Register_WriteMonitorReport(RegisterBuf* readings, int size, FILE* fh) {
  for (int register_ = 0; register_ < RegistersSize; ++register_) {
    bool register_has_changed = false;
    for (range(int, i, 0, size)) {
      if (readings[0][register_] != readings[i][register_]) {
        register_has_changed = true;
        break;
      }
    }

    if (! register_has_changed)
      continue;

    fprintf(fh, "%.2X", register_);
    for (range(int, i, 0, size)) {
      if (options.clearly && i > 0 && readings[i][register_] == readings[i - 1][register_])
        continue;

      if (options.decimal)
        fprintf(fh, ",%d", readings[i][register_]);
      else
        fprintf(fh, ",%.2X", readings[i][register_]);
    }
    fprintf(fh, "\n");
  }
}

static void Register_PrintDump(RegisterBuf* self, bool use_color) {
  RegisterColors colors;

  if (use_color) {
    for (int i = 0; i < RegistersSize; ++i)
      colors[i] = (my[i] == 0x00 ? Console_BoldBlack :
                   my[i] == 0xFF ? Console_BoldGreen :
                                   Console_BoldBlue);

    Register_PrintRegister(self, colors);
    printf("%s", Console_Reset);
  }
  else {
    Register_PrintRegister(self, NULL);
  }
}

static int Register_LoadDump(RegisterBuf* self, FILE* fh) {
  char header[sizeof(RegisterHeader)] = {0};
  fread(header, 1, sizeof(RegisterHeader) - 1, fh);
  if (strcmp(header, RegisterHeader))
    goto error;

  for (int line_no = 0; line_no < 16; ++line_no) {
    char prefix[6] = {0};
    if (fread(prefix, 1, 5, fh) != 5) {
      goto error;
    }

    for (int column_no = 0; column_no < 16; ++column_no) {
      char hex[3] = {0};
      if (fread(hex, 1, 2, fh) != 2)
        goto error;

      char* end;
      const int value = strtoll(hex, &end, 16);
      if (*end != '\0')
        goto error;

      fread(hex, 1, 1, fh);

      const int register_no = (line_no * 16) + column_no;
      my[register_no] = value;
    }
  }

  return NBFC_EXIT_SUCCESS;

error:
  Log_Error("File is not a valid register dump");
  return NBFC_EXIT_FAILURE;
}

struct Args {
  const char* args[64];
  ssize_t count;
};

static void ShellRead(const struct Args* args) {
  int word = 0;
  const char* register_arg = NULL;
  const char* err;

  for (int i = 1; i < args->count; ++i) {
    const char* arg = args->args[i];

    if (arg[0] == '-') {
      if (!strcmp(arg, "-w") || !strcmp(arg, "--word"))
        word = 1;
      else
        return (void) printf("ERR: Invalid option: %s\n", arg);
    }
    else if (! register_arg)
      register_arg = arg;
    else
      return (void) printf("ERR: Too much arguments\n");
  }

  if (! register_arg)
    return (void) printf("ERR: Missing argument (REGISTER)\n");

  int register_ = parse_number(register_arg, 0, word ? 254 : 255, &err);
  if (err)
    return (void) printf("ERR: Argument (REGISTER): %s\n", err);

  if (word) {
    uint16_t value;
    Error* e = ec->ReadWord(register_, &value);
    if (e)
      return (void) printf("ERR: %s\n", err_print_all(e));

    printf("%d\n", value);
  }
  else {
    uint8_t value;
    Error* e = ec->ReadByte(register_, &value);
    if (e)
      return (void) printf("ERR: %s\n", err_print_all(e));

    printf("%d\n", value);
  }
}

static void ShellWrite(const struct Args* args) {
  int word = 0;
  const char* register_arg = NULL;
  const char* value_arg = NULL;
  const char* err;

  for (int i = 1; i < args->count; ++i) {
    const char* arg = args->args[i];

    if (arg[0] == '-') {
      if (!strcmp(arg, "-w") || !strcmp(arg, "--word"))
        word = 1;
      else
        return (void) printf("ERR: Invalid option: %s\n", arg);
    }
    else if (! register_arg)
      register_arg = arg;
    else if (! value_arg)
      value_arg = arg;
    else
      return (void) printf("ERR: Too much arguments\n");
  }

  if (! register_arg)
    return (void) printf("ERR: Missing argument (REGISTER)\n");

  if (! value_arg)
    return (void) printf("ERR: Missing argument (VALUE)\n");

  int register_ = parse_number(register_arg, 0, word ? 254 : 255, &err);
  if (err)
    return (void) printf("ERR: Argument (REGISTER): %s\n", err);

  int value = parse_number(value_arg, 0, word ? 65535 : 255, &err);
  if (err)
    return (void) printf("ERR: Argument (VALUE): %s\n", err);

  if (word) {
    Error* e = ec->WriteWord(register_, value);
    if (e)
      return (void) printf("ERR: %s\n", err_print_all(e));
  }
  else {
    Error* e = ec->WriteByte(register_, value);
    if (e)
      return (void) printf("ERR: %s\n", err_print_all(e));
  }

  printf("OK\n");
}

static void ShellReadAll(struct Args* args) {
  uint8_t values[256];

  for (int register_ = 0; register_ <= 255; ++register_) {
    Error* e = ec->ReadByte(register_, &values[register_]);
    if (e)
      return (void) printf("ERR: %s\n", err_print_all(e));
  }

  printf("%d", values[0]);
  for (int register_ = 1; register_ <= 255; ++register_)
    printf(" %d", values[register_]);
  printf("\n");
}

static void ShellHelp() {
  printf(
    "Available commands: \n"
    "  read [-w|--word] REGISTER\n"
    "  write [-w|--word] REGISTER VALUE\n"
    "  read_all\n"
    "  exit | quit\n"
  );
}

static const char* read_arg(char** line) {
  while (**line == ' ' || **line == '\t')
    ++*line;

  if (!**line)
    return NULL;

  const char* arg = *line;

  ++*line;

  while (**line && **line != ' ' && **line != '\t')
    ++*line;

  if (**line) {
    **line = '\0';
    ++*line;
  }

  return arg;
}

static void read_args(struct Args* args, char** line) {
  args->count = 0;

  while (args->count < ARRAY_SSIZE(args->args)) {
    if (! (args->args[args->count] = read_arg(line)))
      break;

    args->count++;
  }
}

static int Shell() {
  char buffer[16384];
  char* line;
  struct Args args;

  signal(SIGINT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  while (fgets(buffer, sizeof(buffer), stdin)) {
    size_t len = strlen(buffer);

    if (buffer[len - 1] != '\n') {
      printf("ERR: Line too long\n");
      continue;
    }

    buffer[len - 1] = '\0';
    line = buffer;

    read_args(&args, &line);

    if (! args.count);
    else if (!strcmp(args.args[0], "read"))     ShellRead(&args);
    else if (!strcmp(args.args[0], "write"))    ShellWrite(&args);
    else if (!strcmp(args.args[0], "read_all")) ShellReadAll(&args);
    else if (!strcmp(args.args[0], "help"))     ShellHelp();
    else if (!strcmp(args.args[0], "exit"))     return 0;
    else if (!strcmp(args.args[0], "quit"))     return 0;
    else
      printf("ERR: No such command: %s (type `help` for a list of commands)\n", args.args[0]);

    fflush(stdout);
  }

  return 0;
}
