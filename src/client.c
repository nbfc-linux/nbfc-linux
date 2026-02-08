// The data structures returned by nxjson are temporary and are loaded into proper C structs.
// We allocate memory from a pool to avoid malloc() and reduce memory usage.
#define NX_JSON_CALLOC(SIZE) ((nx_json*) NXJSON_Memory_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (NXJSON_Memory_Free((void*) (JSON)))

#include <float.h>    // FLT_MAX
#include <limits.h>   // INT_MAX
#include <locale.h>   // setlocale, LC_NUMERIC
#include <stdio.h>    // printf
#include <string.h>   // strcmp
#include <unistd.h>   // geteuid

#include "help/client.help.h"
#include "log.h"
#include "nbfc.h"
#include "parse_number.h"
#include "parse_double.h"
#include "client/client_global.h"

#include "buffer.c"
#include "log.c"
#include "error.c"
#include "file_utils.c"
#include "model_config.c"
#include "fs_sensors.c"
#include "nvidia.c"
#include "memory.c"
#include "nxjson_memory.c"
#include "process.c"
#include "program_name.c"
#include "protocol.c"
#include "nxjson.c"
#include "nxjson_write.c"
#include "service_config.c"
#include "trace.c"
#include "optparse/optparse.c"
#include "mkdir_p.c"
#include "regex_utils.c"
#include "acpi_analysis.c"
#include "config_rating.c"
#include "config_rating_rules.c"
#include "model_config_utils.c"
#include "client/dmi.c"
#include "client/curl_utils.c"
#include "client/config_files.c"
#include "client/str_functions.c"
#include "client/service_control.c"

const cli99_option main_options[] = {
  {"-h|--help",     Option_Help,       0},
  {"-v|--version",  Option_Version,    0},
  {"command",       Option_Command,    1 | cli99_required_option},
  cli99_options_end()
};

// ============================================================================
// Commands
// ============================================================================

#include "client/cmd_start_stop.c"
#include "client/cmd_status.c"
#include "client/cmd_sensors.c"
#include "client/cmd_config.c"
#include "client/cmd_set.c"
#include "client/cmd_update.c"
#include "client/cmd_show_variable.c"
#include "client/cmd_misc.c"
#include "client/cmd_warranty.c"
#include "client/cmd_support.c"
#include "client/cmd_acpi_dump.c"
#include "client/cmd_rate_config.c"

#define NBFC_CLIENT_COMMANDS \
  o("set",              Set,              SET,              set)           \
  o("status",           Status,           STATUS,           status)        \
  o("start",            Start,            START,            start)         \
  o("stop",             Stop,             STOP,             main)          \
  o("restart",          Restart,          RESTART,          start)         \
  o("sensors",          Sensors,          SENSORS,          sensors)       \
  o("config",           Config,           CONFIG,           config)        \
  o("rate-config",      Rate_Config,      RATE_CONFIG,      rate_config)   \
  o("acpi-dump",        Acpi_Dump,        ACPI_DUMP,        acpi_dump)     \
  o("update",           Update,           UPDATE,           update)        \
  o("wait-for-hwmon",   Wait_For_Hwmon,   WAIT_FOR_HWMON,   main)          \
  o("get-model-name",   Get_Model_Name,   GET_MODEL,        main)          \
  o("complete-fans",    Complete_Fans,    COMPLETE_FANS,    main)          \
  o("complete-sensors", Complete_Sensors, COMPLETE_SENSORS, main)          \
  o("show-variable",    Show_Variable,    SHOW_VARIABLE,    show_variable) \
  o("warranty",         Warranty,         WARRANTY,         main)          \
  o("donate",           Donate,           SUPPORT,          main)          \
  o("support",          Support,          SUPPORT,          support)       \
  o("help",             Help,             HELP,             main)
//  COMMAND             ENUM              HELP TEXT         OPTIONS

enum Command {
#define o(COMMAND, ENUM, HELP, OPTIONS)  Command_ ## ENUM,
  NBFC_CLIENT_COMMANDS
  Command_End
#undef o
};

static const char *HelpTexts[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  CLIENT_ ## HELP ## _HELP_TEXT,
  NBFC_CLIENT_COMMANDS
#undef o
};

static enum Command Command_FromString(const char* s) {
  const char* commands[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  COMMAND,
    NBFC_CLIENT_COMMANDS
#undef o
  };

  for (int i = 0; i < ARRAY_SSIZE(commands); ++i)
    if (!strcmp(commands[i], s))
      return (enum Command) i;

  return Command_End;
}

static const cli99_option *Options[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  OPTIONS ## _options,
  NBFC_CLIENT_COMMANDS
#undef o
};

// ============================================================================
// Main
// ============================================================================

int main(int argc, char *const argv[]) {
  if (geteuid() == 0) {
    mkdir_p(NBFC_CONFIG_DIR, 0755);
    mkdir_p(NBFC_MODEL_CONFIGS_DIR_MUTABLE, 0755);
  }

  if (argc == 1) {
    printf(CLIENT_HELP_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }

  Program_Name_Set(argv[0]);
  setlocale(LC_NUMERIC, "C"); // for json floats

  int o;
  const char* err;
  enum Command cmd = Command_Help;
  cli99 p;
  cli99_Init(&p, argc, argv, main_options, cli99_options_python);
  while ((o = cli99_GetOpt(&p))) {
    switch (o) {

    // ========================================================================
    // General options
    // ========================================================================

    case Option_Help:
      printf("%s", HelpTexts[cmd]);
      return NBFC_EXIT_SUCCESS;

    case Option_Version:
      printf("nbfc " NBFC_VERSION "\n");
      return NBFC_EXIT_SUCCESS;

    // ========================================================================
    // Command
    // ========================================================================

    case Option_Command:
      cmd = Command_FromString(p.optarg);
      if (cmd == Command_End) {
        Log_Error("Invalid command: %s", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      if (cmd == Command_Help) {
        printf("%s", HelpTexts[Command_Help]);
        return NBFC_EXIT_SUCCESS;
      }
      cli99_SetOptions(&p, Options[cmd], false);
      break;

    // ========================================================================
    // Status options
    // ========================================================================

    case Option_Status_All:
      Status_Options.all = true;
      break;

    case Option_Status_Service:
      Status_Options.service = true;
      break;

    case Option_Status_Fan:
      {
        const int fan = parse_number(p.optarg, 0, INT_MAX, &err);
        if (err) {
          Log_Error("%s: %s: %s", "-f|--fan", err, p.optarg);
          return NBFC_EXIT_FAILURE;
        }

        array_of(int)* fans = &Status_Options.fans;
        fans->size++;
        fans->data = Mem_Realloc(fans->data, fans->size * sizeof(int));
        fans->data[fans->size - 1] = fan;
      }
      break;

    case Option_Status_Watch:
      Status_Options.watch = parse_double(p.optarg, 0.1, FLT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", "-w|--watch", err, p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    // ========================================================================
    // Sensors options
    // ========================================================================

    case Option_Sensors_Command:
      Sensors_Options.command = Sensors_Command_FromString(p.optarg);
      if (Sensors_Options.command == Sensors_Command_End) {
        Log_Error("Invalid command: sensors %s", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      if (Sensors_Options.command == Sensors_Command_Set)
        cli99_SetOptions(&p, sensors_set_options, false);

      break;

    case Option_Sensors_Fan:
      Sensors_Options.fan = parse_number(p.optarg, 0, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", "-f|--fan", err, p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    case Option_Sensors_Sensor:
      {
        array_of(str)* sensors = &Sensors_Options.sensors;
        sensors->size++;
        sensors->data = Mem_Realloc(sensors->data, sensors->size * sizeof(str));
        sensors->data[sensors->size - 1] = p.optarg;
      }
      break;

    case Option_Sensors_Algorithm:
      Sensors_Options.algorithm = TemperatureAlgorithmType_FromString(p.optarg);
      if (Sensors_Options.algorithm == TemperatureAlgorithmType_Unset) {
        Log_Error("%s: %s: %s", "-a|--algorithm", "Invalid value", p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    case Option_Sensors_Force:
      Sensors_Options.force = true;
      break;

    // ========================================================================
    // Config options
    // ========================================================================

    case Option_Config_Apply:
      Set_Config_Action(Config_Action_Apply);
      Config_Options.config = p.optarg;
      break;

    case Option_Config_Set:
      Set_Config_Action(Config_Action_Set);
      Config_Options.config = p.optarg;
      break;

    case Option_Config_List:
      Set_Config_Action(Config_Action_List);
      break;

    case Option_Config_Recommend:
      Set_Config_Action(Config_Action_Recommend);
      break;

    // ========================================================================
    // Set options
    // ========================================================================

    case Option_Set_Auto:
      if (Set_Options.speed != -2.0) {
        Log_Error("Options -a|--auto or -s|--speed may only be specified once");
        return NBFC_EXIT_FAILURE;
      }

      Set_Options.speed = -1.0;
      break;

    case Option_Set_Speed:
      if (Set_Options.speed != -2.0) {
        Log_Error("Options -a|--auto or -s|--speed may only be specified once");
        return NBFC_EXIT_FAILURE;
      }

      Set_Options.speed = parse_double(p.optarg, 0, 100, &err);
      if (err) {
        Log_Error("%s: %s: %s", "-s|--speed", err, p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    case Option_Set_Fan:
      if (Set_Options.fan != -1) {
        Log_Error("Option -f|--fan may only be specified once");
        return NBFC_EXIT_FAILURE;
      }

      Set_Options.fan = parse_number(p.optarg, 0, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", "-f|--fan", err, p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    // ========================================================================
    // Update options
    // ========================================================================

    case Option_Update_Parallel:
      Update_Options.parallel = parse_number(p.optarg, 0, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", "-p|--parallel", err, p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    case Option_Update_Quiet:
      Update_Options.quiet = true;
      break;

    // ========================================================================
    // Start/Restart options
    // ========================================================================

    case Option_Start_ReadOnly:
      Start_Options.read_only = true;
      break;

    // ========================================================================
    // Support options
    // ========================================================================

    case Option_Support_Upload_Firmware:
      Support_Options.action = Support_Action_Upload_Firmware;
      break;

    case Option_Support_Print_Command:
      Support_Options.action = Support_Action_Print_Command;
      break;

    // ========================================================================
    // Rate-Config options
    // ========================================================================

    case Option_Rate_Config_File:
      Rate_Config_Options.file = p.optarg;
      break;

    case Option_Rate_Config_DSDT_File:
      Rate_Config_Options.dsdt_file = p.optarg;
      break;

    case Option_Rate_Config_All:
      Rate_Config_Options.all = true;
      break;

    case Option_Rate_Config_Full_Help:
      Rate_Config_Options.full_help = true;
      break;

    case Option_Rate_Config_Json:
      Rate_Config_Options.json = true;
      break;

    case Option_Rate_Config_Print_Rules:
      Rate_Config_Options.print_rules = true;
      break;

    case Option_Rate_Config_Min_Score:
      Rate_Config_Options.min_score_set = true;
      Rate_Config_Options.min_score = parse_double(p.optarg, 0, 10, &err);
      if (err) {
        Log_Error("%s: %s: %s", "-m|--min-score", err, p.optarg);
        return NBFC_EXIT_FAILURE;
      }
      break;

    // ========================================================================
    // Acpi-Dump options
    // ========================================================================

    case Option_Acpi_Dump_File:
      Acpi_Dump_Options.file = p.optarg;
      break;

    case Option_Acpi_Dump_Json:
      Acpi_Dump_Options.json = true;
      break;

    case Option_Acpi_Dump_Command:
      Acpi_Dump_Options.action = AcpiDump_CommandFromString(p.optarg);
      if (Acpi_Dump_Options.action == AcpiDump_Action_None) {
        Log_Error("%s: Invalid command", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    // ========================================================================
    // Show-Variable options
    // ========================================================================

    case Option_ShowVariable_Variable:
      Show_Variable_Options.variable = p.optarg;
      break;

    // ========================================================================
    // Error
    // ========================================================================

    default:
      cli99_ExplainError(&p);
      return NBFC_EXIT_CMDLINE;
    }
  }

  switch (cmd) {
  case Command_Start:             return Start();
  case Command_Stop:              return Stop();
  case Command_Restart:           return Restart();
  case Command_Config:            return Config();
  case Command_Rate_Config:       return RateConfig();
  case Command_Acpi_Dump:         return AcpiDump();
  case Command_Set:               return Set();
  case Command_Status:            return Status();
  case Command_Sensors:           return Sensors();
  case Command_Update:            return Update();
  case Command_Wait_For_Hwmon:    return Wait_For_Hwmon();
  case Command_Get_Model_Name:    return Get_Model_Name();
  case Command_Show_Variable:     return Show_Variable();
  case Command_Complete_Fans:     return Complete_Fans();
  case Command_Complete_Sensors:  return Complete_Sensors();
  case Command_Warranty:          return Warranty();
  case Command_Donate:            return Support();
  case Command_Support:           return Support();
  default:                        return NBFC_EXIT_FAILURE;
  }
}
