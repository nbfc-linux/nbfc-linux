// The data structures returned by nxjson are temporary and are loaded into proper C structs.
// We allocate memory from a pool to avoid malloc() and reduce memory usage.
#define NX_JSON_CALLOC(SIZE) ((nx_json*) NXJSON_Memory_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (NXJSON_Memory_Free((void*) (JSON)))

#define _GNU_SOURCE

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

#include "acpi_call.c"
#include "buffer.c"
#include "log.c"
#include "lua_bindings.c"
#include "error.c"
#include "ec.h"
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
#include "cli99.c"
#include "mkdir_p.c"
#include "regex_utils.c"
#include "acpi_analysis.c"
#include "aml_lexer.c"
#include "aml_parser.c"
#include "aml_analysis.c"
#include "config_analysis.c"
#include "config_rating.c"
#include "config_rating_rules.c"
#include "model_config_utils.c"
#include "model_config_to_json.c"
#include "str_functions.c"
#include "xml2json.c"
#include "client/dmi.c"
#include "client/curl_utils.c"
#include "client/config_files.c"
#include "client/service_control.c"

const EC_VTable* ec = NULL;

const struct cli99_Option Main_CommandLine[] = {
  {"-h|--help",     Option_Help,       cli99_NoArgument      },
  {"-v|--version",  Option_Version,    cli99_NoArgument      },
  {"command",       Option_Command,    cli99_NormalPositional},
  cli99_Options_End()
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
#include "client/cmd_xml2json.c"

#define NBFC_CLIENT_COMMANDS \
  o("set",              Set,              SET,              Set)           \
  o("status",           Status,           STATUS,           Status)        \
  o("start",            Start,            START,            Start)         \
  o("stop",             Stop,             STOP,             Main)          \
  o("restart",          Restart,          RESTART,          Start)         \
  o("sensors",          Sensors,          SENSORS,          Sensors)       \
  o("config",           Config,           CONFIG,           Config)        \
  o("rate-config",      RateConfig,       RATE_CONFIG,      RateConfig)    \
  o("acpi-dump",        AcpiDump,         ACPI_DUMP,        AcpiDump)      \
  o("update",           Update,           UPDATE,           Update)        \
  o("wait-for-hwmon",   Wait_For_Hwmon,   WAIT_FOR_HWMON,   Main)          \
  o("get-model-name",   Get_Model_Name,   GET_MODEL,        Main)          \
  o("complete-fans",    Complete_Fans,    COMPLETE_FANS,    Main)          \
  o("complete-sensors", Complete_Sensors, COMPLETE_SENSORS, Main)          \
  o("show-variable",    ShowVariable,     SHOW_VARIABLE,    ShowVariable)  \
  o("xml2json",         Xml2Json,         XML2JSON,         Xml2Json)      \
  o("warranty",         Warranty,         WARRANTY,         Main)          \
  o("donate",           Donate,           SUPPORT,          Main)          \
  o("support",          Support,          SUPPORT,          Support)       \
  o("help",             Help,             HELP,             Main)          \
  o("faq",              FAQ,              FAQ,              Main)
//  COMMAND             ENUM              HELP TEXT         COMMANDLINE

enum Command {
#define o(COMMAND, ENUM, HELP, OPTIONS)  Command_ ## ENUM,
  NBFC_CLIENT_COMMANDS
  Command_End
#undef o
};

static const char* HelpTexts[] = {
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

static const struct cli99_Option *Options[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  OPTIONS ## _CommandLine,
  NBFC_CLIENT_COMMANDS
#undef o
};

// ============================================================================
// Main
// ============================================================================

int main(int argc, char* const argv[]) {
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

  int64_t o;
  const char* err;
  enum Command cmd = Command_Help;
  struct cli99 p;
  cli99_Init(&p, Main_CommandLine, argv, argc);
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

      p.options = Options[cmd];
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
        const array_size_t fan = (array_size_t) parse_number(p.optarg, 0, INT_MAX, &err);
        if (err) {
          Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
          return NBFC_EXIT_CMDLINE;
        }

        array_of(array_size_t)* fans = &Status_Options.fans;
        bool have_fan = false;

        for_each_array(array_size_t*, fan1, *fans) {
          if (*fan1 == fan) {
            have_fan = true;
            break;
          }
        }

        if (! have_fan) {
          fans->size++;
          array_realloc(array_size_t, *fans, fans->size);
          fans->data[fans->size - 1] = fan;
        }
      }
      break;

    case Option_Status_Watch:
      Status_Options.watch = (float) parse_double(p.optarg, 0.1, FLT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    // ========================================================================
    // Sensors options
    // ========================================================================

    case Option_Sensors_Command:
      Sensors_Options.command = Sensors_Command_FromString(p.optarg);
      if (Sensors_Options.command == Sensors_Command_None) {
        Log_Error("Invalid command: sensors %s", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      if (Sensors_Options.command == Sensors_Command_Set)
        p.options = Sensors_Set_CommandLine;

      break;

    case Option_Sensors_Fan:
      Sensors_Options.fan = (array_size_t) parse_number(p.optarg, 0, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    case Option_Sensors_Sensor:
      {
        array_of(str)* sensors = &Sensors_Options.sensors;
        sensors->size++;
        array_realloc(str, *sensors, sensors->size);
        sensors->data[sensors->size - 1] = p.optarg;
      }
      break;

    case Option_Sensors_Algorithm:
      Sensors_Options.algorithm = TemperatureAlgorithmType_FromString(p.optarg);
      if (Sensors_Options.algorithm == TemperatureAlgorithmType_Unset) {
        Log_Error("%s: %s: %s", p.option->optstring, "Invalid value", p.optarg);
        return NBFC_EXIT_CMDLINE;
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

    case Option_Config_Yes:
      Config_Options.yes = true;
      break;

    // ========================================================================
    // Set options
    // ========================================================================

    case Option_Set_Auto:
      if (Set_Options.speed != -2.0f) {
        Log_Error("Options -a|--auto or -s|--speed may only be specified once");
        return NBFC_EXIT_CMDLINE;
      }

      Set_Options.speed = -1.0f;
      break;

    case Option_Set_Speed:
      if (Set_Options.speed != -2.0f) {
        Log_Error("Options -a|--auto or -s|--speed may only be specified once");
        return NBFC_EXIT_CMDLINE;
      }

      Set_Options.speed = (float) parse_double(p.optarg, 0, 100, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    case Option_Set_Fan:
      if (Set_Options.fan != -1) {
        Log_Error("Option %s may only be specified once", p.option->optstring);
        return NBFC_EXIT_CMDLINE;
      }

      Set_Options.fan = (int) parse_number(p.optarg, 0, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    // ========================================================================
    // Update options
    // ========================================================================

    case Option_Update_Parallel:
      Update_Options.parallel = (int) parse_number(p.optarg, 1, INT_MAX, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
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

    case Option_Support_Create_Archive:
      Support_Options.action = Support_Action_Create_Archive;
      Support_Options.archive_file = p.optarg;
      break;

    // ========================================================================
    // Rate-Config options
    // ========================================================================

    case Option_RateConfig_All:
      RateConfig_SetAction(RateConfig_Action_RateAll, p.option->optstring);
      break;

    case Option_RateConfig_File:
      RateConfig_SetAction(RateConfig_Action_RateFile, p.option->optstring);
      RateConfig_Options.file = p.optarg;
      break;

    case Option_RateConfig_Input:
      RateConfig_SetAction(RateConfig_Action_RateFromFile, p.option->optstring);
      RateConfig_Options.input_file = p.optarg;
      break;

    case Option_RateConfig_Full_Help:
      RateConfig_SetAction(RateConfig_Action_PrintFullHelp, p.option->optstring);
      break;

    case Option_RateConfig_Print_Rules:
      RateConfig_SetAction(RateConfig_Action_PrintRules, p.option->optstring);
      break;

    case Option_RateConfig_DSDT_File:
      if (RateConfig_Options.dsdt_files_size >= ACPI_ANALYSIS_MAX_AML_FILES) {
        Log_Error("%s: Too many files given", p.option->optstring);
        return NBFC_EXIT_CMDLINE;
      }

      RateConfig_Options.dsdt_files[RateConfig_Options.dsdt_files_size++] = p.optarg;
      break;

   case Option_RateConfig_DSDT_Dir:
      RateConfig_Options.dsdt_dir = p.optarg;
      break;

   case Option_RateConfig_Rules:
      RateConfig_Options.rules_file = p.optarg;
      break;

   case Option_RateConfig_No_Download:
      RateConfig_Options.no_download = true;
      break;

    case Option_RateConfig_Json:
      RateConfig_Options.json = true;
      break;

    case Option_RateConfig_Unverified:
      RateConfig_Options.unverified = true;
      break;

    case Option_RateConfig_Min_Score:
      RateConfig_Options.min_score_set = true;
      RateConfig_Options.min_score = (float) parse_double(p.optarg, 0, 10, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    case Option_RateConfig_Bad:
      RateConfig_Options.filter = RateConfig_FilterBadOnly;
      break;

    case Option_RateConfig_Quiet:
      if (RateConfig_Options.style)
        RateConfig_Options.style--;
      break;

    case Option_RateConfig_FanCount:
      RateConfig_Options.fan_count = (uint8_t) parse_number(p.optarg, 0, 255, &err);
      if (err) {
        Log_Error("%s: %s: %s", p.option->optstring, err, p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    // ========================================================================
    // Acpi-Dump options
    // ========================================================================

    case Option_AcpiDump_DSDT_File:
      if (AcpiDump_Options.files_size >= ACPI_ANALYSIS_MAX_AML_FILES) {
        Log_Error("%s: Too many files given", p.option->optstring);
        return NBFC_EXIT_CMDLINE;
      }

      AcpiDump_Options.files[AcpiDump_Options.files_size++] = p.optarg;
      break;

    case Option_AcpiDump_DSDT_Dir:
      AcpiDump_Options.dir = p.optarg;
      break;

    case Option_AcpiDump_Json:
      AcpiDump_Options.json = true;
      break;

    case Option_AcpiDump_Unverified:
      AcpiDump_Options.unverified = true;
      break;

    case Option_AcpiDump_Command:
      AcpiDump_Options.action = AcpiDump_CommandFromString(p.optarg);
      if (AcpiDump_Options.action == AcpiDump_Action_None) {
        Log_Error("Invalid command: %s", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;

    // ========================================================================
    // Show-Variable options
    // ========================================================================

    case Option_ShowVariable_Variable:
      ShowVariable_Options.variable = p.optarg;
      break;

    // ========================================================================
    // Xml2Json options
    // ========================================================================

    case Option_Xml2Json_File:
      Xml2Json_Options.file = p.optarg;
      break;

    // ========================================================================
    // Error
    // ========================================================================

    default:
      Log_Error("%s: %s", cli99_StrError(p.error), p.error_cause);
      return NBFC_EXIT_CMDLINE;
    }
  }

  switch (cmd) {
  case Command_Start:             return Start();
  case Command_Stop:              return Stop();
  case Command_Restart:           return Restart();
  case Command_Config:            return Config();
  case Command_RateConfig:        return RateConfig();
  case Command_AcpiDump:          return AcpiDump();
  case Command_Set:               return Set();
  case Command_Status:            return Status();
  case Command_Sensors:           return Sensors();
  case Command_Update:            return Update();
  case Command_Wait_For_Hwmon:    return WaitForHwmon();
  case Command_Get_Model_Name:    return GetModelName();
  case Command_ShowVariable:      return ShowVariable();
  case Command_Complete_Fans:     return CompleteFans();
  case Command_Complete_Sensors:  return CompleteSensors();
  case Command_Warranty:          return Warranty();
  case Command_Donate:            return Support();
  case Command_Support:           return Support();
  case Command_Xml2Json:          return Xml2Json();
  case Command_FAQ:               return FAQ();
  default:                        return NBFC_EXIT_FAILURE;
  }
}
