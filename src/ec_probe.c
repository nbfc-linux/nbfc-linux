// The data structures returned by nxjson are temporary and are loaded into proper C structs.
// We allocate memory from a pool to avoid malloc() and reduce memory usage.
#define NX_JSON_CALLOC(SIZE) ((nx_json*) NXJSON_Memory_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (NXJSON_Memory_Free((void*) (JSON)))

#define _XOPEN_SOURCE  500 // unistd.h: export pwrite()/pread()
#define _DEFAULT_SOURCE    // endian.h: export htole16()/le16toh()
#define _GNU_SOURCE

#include "nbfc.h"
#include "macros.h"
#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "model_config.h"
#include "cli99.h"
#include "parse_number.h"
#include "parse_unumber.h"
#include "parse_double.h"
#include "help/ec_probe.help.h"
#include "log.h"

#include <float.h>   // FLT_MAX
#include <stdbool.h> // bool
#include <stdio.h>   // printf
#include <stdint.h>  // uint8_t, uint16_t, uint64_t
#include <string.h>  // strcmp, strlen, strrchr
#include <limits.h>  // INT_MAX, UINT64_MAX
#include <locale.h>  // setlocale, LC_NUMERIC
#include <signal.h>  // signal, SIGINT, SIGTERM

#include "error.c"          // src
#include "ec.c"             // src

#if ENABLE_EC_DEV_PORT
#include "ec_linux.c"       // src
#endif

#if ENABLE_EC_SYS || ENABLE_EC_ACPI
#include "ec_sys_linux.c"   // src
#endif

#if ENABLE_EC_DUMMY
#include "ec_dummy.c"       // src
#endif

#include "acpi_call.c"      // src
#include "buffer.c"         // src
#include "cli99.c"          // src
#include "file_utils.c"     // src
#include "fs_sensors.c"     // src
#include "io_utils.c"       // src
#include "log.c"            // src
#include "lua_bindings.c"   // src
#include "memory.c"         // src
#include "model_config.c"   // src
#include "nvidia.c"         // src
#include "nxjson_memory.c"  // src
#include "nxjson.c"         // src
#include "program_name.c"   // src
#include "process.c"        // src
#include "spearman.c"       // src
#include "str_functions.c"  // src
#include "trace.c"          // src
#include "vfio.c"           // src

#define REGISTERS_SIZE 256

typedef struct RegisterReadings RegisterReadings;
struct RegisterReadings {
  uint8_t readings[REGISTERS_SIZE];
  float cpu_temp;
  float gpu_temp;
};

typedef const char* RegisterColors[REGISTERS_SIZE];

// Allocate enough space for register entries. This simplifies
// the code and avoids the need for dynamic reallocation.
static RegisterReadings Registers_Log[32768];

const EC_VTable* ec;
static volatile int quit;

typedef enum NBFC_PACKED_ENUM {
  Option_None = 0,
  Option_Help,
  Option_Version,
  Option_EmbeddedController,
  Option_Command,
  Option_Word,
  Option_Dry,
  Option_Register,
  Option_Value,
  Option_BitOffset,
  Option_BitValue,
  Option_Format,
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
  Option_Map,
  Option_Watch,
  Option_NoRestore,
  Option_Cpu,
  Option_Gpu,
  Option_RegisterColor,
  Option_CpuColor,
  Option_GpuColor,
} Option;

typedef enum NBFC_PACKED_ENUM {
  ColorModeAuto = 0,
  ColorModeEnable,
  ColorModeDisable,
} ColorMode;

static struct {
  int         timespan;
  float       interval;
  const char* report;
  const char* file;
  const char* map;
  bool        clearly;
  bool        decimal;
  bool        dry;
  bool        no_restore;
  const char* register_ref;
  const char* watch_ref;
  uint8_t     register_;
  uint8_t     watch;
  uint16_t    value;
  uint8_t     bit_offset;
  uint8_t     bit_value;
  bool        use_word;
  ColorMode   color_mode;
  char        format;
  const char* acpi_call_method;
  uint64_t    acpi_call_args[8];
  int         acpi_call_args_size;
  bool        cpu;
  bool        gpu;
  const char* register_color;
  const char* cpu_color;
  const char* gpu_color;
  uint64_t    _set;
} options = {0};

static void Initialize_EC(void) {
  static bool initialized = false;
  if (initialized)
    return;

  if (ec == NULL) {
    Error e = EC_FindWorking(&ec);
    e_die();
  }

  Error e = ec->Open();
  e_die();

  initialized = true;
}

static inline Error Registers_FromEC(RegisterReadings* readings) {
  Error e;

  for (size_t i = 0; i < REGISTERS_SIZE; i++) {
    e = ec->ReadByte((uint8_t) i, &readings->readings[i]);
    if (e)
      return e;
  }

  return err_success();
}

static inline Error Registers_ToEC(RegisterReadings* readings) {
  Error e;

  for (size_t i = 0; i < REGISTERS_SIZE; ++i) {
    e = ec->WriteByte((uint8_t) i, readings->readings[i]);
    if (e)
      return e;
  }

  return err_success();
}

static const struct cli99_Option Main_CommandLine[] = {
  {"-e|--embedded-controller", Option_EmbeddedController, cli99_RequiredArgument},
  {"-h|--help",                Option_Help,               cli99_NoArgument      },
  {"--version",                Option_Version,            cli99_NoArgument      },
  {"command",                  Option_Command,            cli99_NormalPositional},
  cli99_Options_End()
};

#include "probe/map.c"
#include "probe/cmd_acpi_call.c"
#include "probe/cmd_dump_load.c"
#include "probe/cmd_monitor.c"
#include "probe/cmd_poke_scan.c"
#include "probe/cmd_read_write.c"
#include "probe/cmd_shell.c"

#define NBFC_EC_PROBE_COMMANDS \
  o("read",         Read,         READ,         Read)          \
  o("write",        Write,        WRITE,        Write)         \
  o("read_bit",     ReadBit,      READ_BIT,     ReadBit)       \
  o("write_bit",    WriteBit,     WRITE_BIT,    WriteBit)      \
  o("acpi_call",    AcpiCall,     ACPI_CALL,    AcpiCall)      \
  o("dump",         Dump,         DUMP,         Dump)          \
  o("load",         Load,         LOAD,         Load)          \
  o("watch",        Watch,        WATCH,        Watch)         \
  o("monitor",      Monitor,      MONITOR,      Monitor)       \
  o("graph",        Graph,        GRAPH,        Graph)         \
  o("evaluate",     Evaluate,     EVALUATE,     Evaluate)      \
  o("poke",         Poke,         POKE,         Poke)          \
  o("scan",         Scan,         SCAN,         Scan)          \
  o("shell",        Shell,        SHELL,        Main)          \
  o("help",         Help,         HELP,         Main)          \
//  COMMAND         ENUM          HELP TEXT     COMMANDLINE

enum Command {
#define o(COMMAND, ENUM, HELP, OPTIONS)  Command_ ## ENUM,
  NBFC_EC_PROBE_COMMANDS
  Command_End
#undef o
};

static const char* HelpTexts[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  EC_PROBE_ ## HELP ## _HELP_TEXT,
  NBFC_EC_PROBE_COMMANDS
#undef o
};

static const char* CommandNames[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  COMMAND,
  NBFC_EC_PROBE_COMMANDS
#undef o
};

static enum Command Command_FromString(const char* s) {
  for (int i = 0; i < ARRAY_SSIZE(CommandNames); ++i)
    if (!strcmp(CommandNames[i], s))
      return (enum Command) i;

  return Command_End;
}

static const struct cli99_Option* Options[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  OPTIONS ## _CommandLine,
  NBFC_EC_PROBE_COMMANDS
#undef o
};

static void RegisterLookup(const char* ref, uint8_t* out) {
  Error e;
  const char* err;

  if (ref[0] >= '0' && ref[0] <= '9') {
    *out = (uint8_t) parse_number(ref, 0, 255, &err);
    if (err) {
      Log_Error("Register: %s", err);
      exit(NBFC_EXIT_CMDLINE);
    }
    return;
  }

  if (! options.map) {
    Log_Error("Register: %s: Not an integer and no -m|--map provided", ref);
    exit(NBFC_EXIT_CMDLINE);
  }

  e = Map_Load(options.map);
  if (e) {
    Log_Error("%s: %s: %s", "-m|--map", options.map, err_print_all(e));
    exit(NBFC_EXIT_FAILURE);
  }

  if (! Map_LookupRegister(ref, out)) {
    Log_Error("Register: %s: Not found in map file", ref);
    exit(NBFC_EXIT_CMDLINE);
  }
}

static void RegisterResolve(void) {
  RegisterLookup(options.register_ref, &options.register_);
  if (options.watch_ref)
    RegisterLookup(options.watch_ref, &options.watch);
}

static void Handle_Signal(int sig) {
  quit = sig;
}

int main(int argc, char* const argv[]) {
  if (argc == 1) {
    printf(EC_PROBE_HELP_HELP_TEXT, argv[0]);
    return NBFC_EXIT_CMDLINE;
  }

  Program_Name_Set(argv[0]);
  setlocale(LC_NUMERIC, "C"); // for parsing floats

  options.interval = 0.5;
  ec = NULL;
  enum Command cmd = Command_Help;

  struct cli99 p;
  cli99_Init(&p, Main_CommandLine, argv, argc);

  int64_t o;
  const char* err;
  while ((o = cli99_GetOpt(&p))) {
    if (o == -1) {
      Log_Error("%s: %s", cli99_StrError(p.error), p.error_cause);
      return NBFC_EXIT_CMDLINE;
    }

    options._set |= (1ULL << o);

    switch (o) {
    case Option_Command:
      cmd = Command_FromString(p.optarg);

      if (cmd == Command_End) {
        Log_Error("Invalid command: %s", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      if (cmd == Command_Help) {
        printf(EC_PROBE_HELP_HELP_TEXT, argv[0]);
        return NBFC_EXIT_SUCCESS;
      }

      p.options = Options[cmd];
      break;
    case Option_Register:
      options.register_ref = p.optarg;
      break;
    case Option_Value:
      options.value = (uint16_t) parse_number(p.optarg, 0, 65535, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Help:      printf(HelpTexts[cmd], argv[0]);         return 0;
    case Option_Version:   printf("ec_probe " NBFC_VERSION "\n");   return 0;
    case Option_Clearly:   options.clearly = true;                  break;
    case Option_Decimal:   options.decimal = true;                  break;
    case Option_Word:      options.use_word = true;                 break;
    case Option_Dry:       options.dry = true;                      break;
    case Option_Map:       options.map = p.optarg;                  break;
    case Option_Watch:     options.watch_ref = p.optarg;            break;
    case Option_NoRestore: options.no_restore = true;               break;
    case Option_Report:    options.report   = p.optarg;             break;
    case Option_Color:     options.color_mode = ColorModeEnable;    break;
    case Option_NoColor:   options.color_mode = ColorModeDisable;   break;
    case Option_File:      options.file = p.optarg;                 break;
    case Option_EmbeddedController:
      switch (EmbeddedControllerType_FromString(p.optarg)) {
#if ENABLE_EC_SYS
        case EmbeddedControllerType_ECSysLinux:     ec = &EC_SysLinux_VTable;      break;
#endif
#if ENABLE_EC_ACPI
        case EmbeddedControllerType_ECSysLinuxACPI: ec = &EC_SysLinux_ACPI_VTable; break;
#endif
#if ENABLE_EC_DEV_PORT
        case EmbeddedControllerType_ECLinux:        ec = &EC_Linux_VTable;         break;
#endif
#if ENABLE_EC_DUMMY
        case EmbeddedControllerType_ECDummy:        ec = &EC_Dummy_VTable;         break;
#endif
        default:
          Log_Error("%s: Invalid value: %s", p.option->optstring, p.optarg);
          return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Timespan:
      options.timespan = (int) parse_number(p.optarg, 1, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Interval:
      options.interval = (float) parse_double(p.optarg, 0.1, FLT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_AcpiCallMethod:
      options.acpi_call_method = p.optarg;
      break;
    case Option_AcpiCallArgument:
      options.acpi_call_args[options.acpi_call_args_size++] = parse_unumber(p.optarg, 0, UINT64_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_BitOffset:
      options.bit_offset = (uint8_t) parse_unumber(p.optarg, 0, 7, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_BitValue:
      options.bit_value = (uint8_t) parse_unumber(p.optarg, 0, 1, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, p.optarg, err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case Option_Format:
      if (strlen(p.optarg) != 1 || !strrchr("bBdDxX", p.optarg[0])) {
        Log_Error("%s: %s: Invalid format", p.option->optstring, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      options.format = p.optarg[0];
      break;
    case Option_Cpu:
      options.cpu = true;
      break;
    case Option_Gpu:
      options.gpu = true;
      break;
    case Option_CpuColor:
      options.cpu_color = p.optarg;
      break;
    case Option_GpuColor:
      options.gpu_color = p.optarg;
      break;
    case Option_RegisterColor:
      options.register_color = p.optarg;
      break;
    }
  }

#define CHECK_REQUIRED_ARGUMENT(ENUM, HUMAN_READABLE)                         \
  do {                                                                        \
    if (! (options._set & (1ULL << ENUM))) {                                  \
      Log_Error("Argument required: %s", HUMAN_READABLE);                     \
      return NBFC_EXIT_CMDLINE;                                               \
    }                                                                         \
  } while (0)

  switch (cmd) {
  case Command_Load:
    CHECK_REQUIRED_ARGUMENT(Option_File, "file");
    break;

  case Command_Read:
    CHECK_REQUIRED_ARGUMENT(Option_Register, "register");
    break;

  case Command_Write:
    CHECK_REQUIRED_ARGUMENT(Option_Register, "register");
    CHECK_REQUIRED_ARGUMENT(Option_Value, "value");
    break;

  case Command_ReadBit:
    CHECK_REQUIRED_ARGUMENT(Option_Register, "register");
    CHECK_REQUIRED_ARGUMENT(Option_BitOffset, "bit_offset");
    break;

  case Command_WriteBit:
    CHECK_REQUIRED_ARGUMENT(Option_Register, "register");
    CHECK_REQUIRED_ARGUMENT(Option_BitOffset, "bit_offset");
    CHECK_REQUIRED_ARGUMENT(Option_BitValue, "bit_value");
    break;

  case Command_AcpiCall:
    CHECK_REQUIRED_ARGUMENT(Option_AcpiCallMethod, "method");
    break;

  case Command_Graph:
    CHECK_REQUIRED_ARGUMENT(Option_File, "file");
    break;

  case Command_Evaluate:
    CHECK_REQUIRED_ARGUMENT(Option_File, "file");
    break;

  case Command_Poke:
    CHECK_REQUIRED_ARGUMENT(Option_Register, "register");
    CHECK_REQUIRED_ARGUMENT(Option_Value, "value");
    break;

  case Command_Scan:
    CHECK_REQUIRED_ARGUMENT(Option_Register, "register");
    break;

  default:
    break;
  }

#undef CHECK_REQUIRED_ARGUMENT

  if (options.register_ref)
    RegisterResolve();

  signal(SIGINT,  Handle_Signal);
  signal(SIGTERM, Handle_Signal);

  switch (cmd) {
  case Command_AcpiCall: return AcpiCall();
  case Command_Read:     return Read();
  case Command_Write:    return Write();
  case Command_ReadBit:  return ReadBit();
  case Command_WriteBit: return WriteBit();
  case Command_Dump:     return Dump();
  case Command_Load:     return Load();
  case Command_Watch:    return Watch();
  case Command_Monitor:  return Monitor();
  case Command_Graph:    return Graph();
  case Command_Evaluate: return Evaluate();
  case Command_Poke:     return Poke();
  case Command_Scan:     return Scan();
  case Command_Shell:    return Shell();
  default:               return NBFC_EXIT_FAILURE;
  }
}
