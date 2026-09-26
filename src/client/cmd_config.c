#include <errno.h>        // errno
#include <stdio.h>        // printf, fprintf
#include <stdlib.h>       // exit, realpath, qsort
#include <string.h>       // strrchr, strerror
#include <unistd.h>       // isatty
#include <linux/limits.h> // PATH_MAX

#include "../dmi.h"
#include "../nbfc.h"
#include "../log.h"
#include "../macros.h"
#include "../service_config.h"
#include "../help/client.help.h"

#include "check_root.h"
#include "config_files.h"
#include "client_global.h"
#include "service_control.h"

#define RECOMMENDED_CONFIG_MATCH_THRESHOLD 0.7f

#define RECOMMENDED_WARNING \
  "\n"                                                                         \
  "                   ====================================\n"                  \
  "                   WARNING: This feature is deprecated!\n"                  \
  "                   ====================================\n"                  \
  "\n"                                                                         \
  "The `nbfc config --recommend` feature is included in NBFC-Linux because\n"  \
  "it existed in the original NBFC project.\n"                                 \
  "\n"                                                                         \
  "However, this feature can be *dangerous*.\n"                                \
  "\n"                                                                         \
  "The recommendation mechanism is based solely on loose string matching\n"    \
  "of laptop model names. A similar model name does not imply that the\n"      \
  "configuration can be used safely.\n"                                        \
  "\n"                                                                         \
  "In the worst case, a misapplied configuration may write to battery\n"       \
  "control registers, potentially causing permanent battery damage\n"          \
  "or hardware failure.\n"                                                     \
  "\n"                                                                         \
  "If you still want to get a list of similar named model configurations,\n"   \
  "re-run this command with `-y|--yes`.\n"                                     \
  "\n"                                                                         \
  "The recommended way to find configurations that are safe on your system\n"  \
  "is to run `sudo nbfc rate-config -a`.\n"                                    \
  "\n"                                                                         \
  ""

#define CONFIG_APPLY_NOTICE \
  "The -a|--apply option has been removed.\n"                                  \
  "\n"                                                                         \
  "For setting up the service, use the following commands:\n"                  \
  "  $ sudo nbfc config --set \"CONFIG\"\n"                                    \
  "  $ sudo nbfc restart --read-only\n"                                        \
  "\n"                                                                         \
  "If the configuration works in read-only mode, you can restart the service\n"\
  "in write-mode:\n"                                                           \
  "  $ sudo nbfc restart\n"                                                    \
  ""

#define CONFIG_SET_NOTICE \
  "Configuration has been set successfully.\n"                                 \
  "\n"                                                                         \
  "To test the configuration use:\n"                                           \
  "  $ sudo nbfc restart --read-only\n"                                        \
  "\n"                                                                         \
  "Once you verified the configuration, start the service in write-mode:\n"    \
  "  $ sudo nbfc restart"                                                      \
  ""

#define CONFIG_ADVANCED_NOTICE \
  "This configuration contains more than one fan configuration without a\n"    \
  "configured temperature sensor. The default temperature sensor (@CPU) will\n"\
  "be used for these fans.\n"                                                  \
  "\n"                                                                         \
  "If this configuration contains a GPU fan, you may want to change its\n"     \
  "temperature sensor to @GPU using `nbfc sensors`."                           \
  ""

typedef enum NBFC_PACKED_ENUM {
  Config_Action_None = 0,
  Config_Action_Apply,
  Config_Action_Set,
  Config_Action_List,
  Config_Action_Recommend
} Config_Action;

const struct cli99_Option Config_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-l|--list",      Option_Config_List,      cli99_NoArgument      },
  {"-r|--recommend", Option_Config_Recommend, cli99_NoArgument      },
  {"-s|--set",       Option_Config_Set,       cli99_RequiredArgument},
  {"-a|--apply",     Option_Config_Apply,     cli99_RequiredArgument},
  {"-y|--yes",       Option_Config_Yes,       cli99_NoArgument      },
  cli99_Options_End()
};

struct {
  Config_Action action;
  const char* config;
  bool yes;
} Config_Options = {0};

void Set_Config_Action(Config_Action action) {
  if (Config_Options.action && Config_Options.action != action) {
    Log_Error("Options --apply, --set, --list and --recommend are mutually exclusive");
    exit(NBFC_EXIT_CMDLINE);
  }

  Config_Options.action = action;
}

static int Config_List(void) {
  array_of(ConfigFile) files = List_All_Configs();

  qsort(files.data, files.size, sizeof(ConfigFile), ConfigFile_CompareByName);

  for_each_array(ConfigFile*, file, files) {
    printf("%s\n", file->config_name);
  }

  return NBFC_EXIT_SUCCESS;
}

static int Config_Recommend(void) {
  Error e;
  char model_name[DMI_MAX_MODEL_NAME_LEN];

  if (isatty(STDOUT_FILENO) && !Config_Options.yes) {
    fprintf(stderr, "%s", RECOMMENDED_WARNING);
    return NBFC_EXIT_FAILURE;
  }

  e = DMI_GetModelName(model_name, sizeof(model_name));
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  array_of(ConfigFile) files = List_Recommended_Configs();
  char* config = Get_Supported_Config(&files, model_name);

  if (config) {
    fprintf(stderr, "Found supported config:\n");
    printf("%s\n", config);
    return NBFC_EXIT_SUCCESS;
  }

  bool have_match = false;
  for_each_array(ConfigFile*, file, files) {
    if (file->diff >= RECOMMENDED_CONFIG_MATCH_THRESHOLD) {
      have_match = true;
      printf("%s\n", file->config_name);
    }
  }

  if (! have_match) {
    Log_Error("No recommended configuration files found");
  }

  return NBFC_EXIT_SUCCESS;
}

static void Config_LoadAndValidateModelConfig(ModelConfig* model_config, const char* config) {
  Error e;
  Trace trace = {0};
  char path[PATH_MAX];
  const LogLevel old_log_level = Log_LogLevel;

  e = ModelConfig_FindAndLoad(model_config, path, config);
  if (e) {
    Log_Error("%s: %s", path, err_print_all(e));
    exit(NBFC_EXIT_FAILURE);
  }

  Trace_Push(&trace, "%s", path);
  Log_LogLevel = LogLevel_Quiet;
  e = ModelConfig_Validate(&trace, model_config);
  Log_LogLevel = old_log_level;
  if (e) {
    Log_Error("%s: %s", path, err_print_all(e));
    exit(NBFC_EXIT_FAILURE);
  }
}

static bool Config_ModelConfigNeedsAdvancedConfiguration(ModelConfig* model_config) {
  if (model_config->FanConfigurations.size == 1)
    return false;

  for_each_array(FanConfiguration*, fan_config, model_config->FanConfigurations) {
    if (! fan_config->isset.Sensors)
      return true;

    if (fan_config->Sensors.size == 0)
      return true;
  }

  return false;
}

static int Config_Set(void) {
  check_root();

  Error e;
  char* config;
  array_of(ConfigFile) files = List_All_Configs();
  ModelConfig model_config = {0};
  ServiceConfig service_config = {0};

  // "auto" ===================================================================
  if (! str_cmp_ignorecase(Config_Options.config, "auto")) {
    char model_name[DMI_MAX_MODEL_NAME_LEN];

    e = DMI_GetModelName(model_name, sizeof(model_name));
    if (e) {
      Log_Error("%s", err_print_all(e));
      return NBFC_EXIT_FAILURE;
    }

    config = Get_Supported_Config(&files, model_name);

    if (! config) {
      Log_Error("No config found to apply automatically");
      return NBFC_EXIT_FAILURE;
    }
  }

  // Filename, not a path =====================================================
  else if (! strrchr(Config_Options.config, '/')) {
    config = Mem_Strdup(Config_Options.config);

    char* dot = strrchr(config, '.');
    if (dot && !str_cmp_ignorecase(dot, ".json"))
      *dot = '\0';

    ConfigFile* found = ConfigFiles_FindLoose(&files, config);
    if (! found) {
      Log_Error("No such configuration available: %s", config);
      return NBFC_EXIT_FAILURE;
    }

    config = Mem_Strdup(found->config_name);
  }

  // Path =====================================================================
  else {
    config = realpath(Config_Options.config, NULL);

    if (! config) {
      Log_Error("Failed to resolve path \"%s\": %s", Config_Options.config, strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
  }

  // Before setting the config, load and validate it ==========================
  Config_LoadAndValidateModelConfig(&model_config, config);

  // Update the service config ================================================
  ServiceConfig_Load(&service_config);
  service_config.SelectedConfigId = config;
  service_config.isset.SelectedConfigId = true;
  e = ServiceConfig_Write(&service_config, NBFC_SERVICE_CONFIG);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // Print notices ============================================================
  printf("%s\n", CONFIG_SET_NOTICE);
  if (Config_ModelConfigNeedsAdvancedConfiguration(&model_config))
    printf("\n%s\n", CONFIG_ADVANCED_NOTICE);

  return NBFC_EXIT_SUCCESS;
}

static int Config_Apply(void) {
  printf("%s\n", CONFIG_APPLY_NOTICE);
  return NBFC_EXIT_FAILURE;
}

int Config(void) {
  switch (Config_Options.action) {
  case Config_Action_List:      return Config_List();
  case Config_Action_Recommend: return Config_Recommend();
  case Config_Action_Set:       return Config_Set();
  case Config_Action_Apply:     return Config_Apply();
  default:
    printf("%s", CLIENT_CONFIG_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }
}
