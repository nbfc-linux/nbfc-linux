#include <errno.h>        // errno
#include <stdio.h>        // printf, fprintf, snprintf
#include <stdlib.h>       // exit, realpath, qsort
#include <string.h>       // strcmp, strrchr, strerror
#include <unistd.h>       // access, F_OK
#include <linux/limits.h> // PATH_MAX

#include "../nbfc.h"
#include "../log.h"
#include "../macros.h"
#include "../service_config.h"
#include "../help/client.help.h"

#include "dmi.h"
#include "check_root.h"
#include "service_control.h"
#include "config_files.h"
#include "client_global.h"

enum Config_Action {
  Config_Action_None = 0,
  Config_Action_Apply,
  Config_Action_Set,
  Config_Action_List,
  Config_Action_Recommend
};

const cli99_option config_options[] = {
  cli99_include_options(&main_options),
  {"-l|--list",      Option_Config_List,      0},
  {"-r|--recommend", Option_Config_Recommend, 0},
  {"-s|--set",       Option_Config_Set,       1},
  {"-a|--apply",     Option_Config_Apply,     1},
  cli99_options_end()
};

struct {
  enum Config_Action action;
  const char* config;
} Config_Options = {0};

void Set_Config_Action(enum Config_Action action) {
  if (Config_Options.action && Config_Options.action != action) {
    Log_Error("Options --apply, --set, --list and --recommend are mutually exclusive\n");
    exit(NBFC_EXIT_CMDLINE);
  }

  Config_Options.action = action;
}

int List() {
  array_of(ConfigFile) files = List_All_Configs();

  qsort(files.data, files.size, sizeof(ConfigFile), compare_config_by_name);

  for_each_array(ConfigFile*, file, files) {
    printf("%s\n", file->config_name);
  }

  return NBFC_EXIT_SUCCESS;
}

int Recommend() {
  const char* model_name = DMI_Get_Model_Name();
  array_of(ConfigFile) files = List_Recommended_Configs();
  char* config = Get_Supported_Config(&files, model_name);

  if (config) {
    fprintf(stderr, "Found supported config:\n");
    printf("%s\n", config);
    return NBFC_EXIT_SUCCESS;
  }

  fprintf(stderr,
    "\n"
    "[!] NOTE:\n"
    "[!] This output is based solely on comparing your model name\n"
    "[!] with the configuration file names.\n"
    "[!] This recommendation does not imply any further validation\n"
    "[!] or significance beyond the string matching of the model\n"
    "[!] and configuration names.\n"
    "\n");

  bool have_match = 0;
  for_each_array(ConfigFile*, file, files) {
    if (file->diff >= RecommendedConfigMatchThreshold) {
      have_match = 1;
      printf("%s\n", file->config_name);
    }
  }

  if (! have_match) {
    Log_Error("No recommended configuration files found\n");
  }

  return NBFC_EXIT_SUCCESS;
}

int Set_Or_Apply() {
  check_root();
  char *config;
  array_of(ConfigFile) files = List_All_Configs();

  // "auto" ===================================================================
  if (! strcmp(Config_Options.config, "auto")) {
    config = Get_Supported_Config(&files, DMI_Get_Model_Name());

    if (! config) {
      Log_Error("No config found to apply automatically\n");
      return NBFC_EXIT_FAILURE;
    }
  }

  // Filename, not a path =====================================================
  else if (! strrchr(Config_Options.config, '/')) {
    config = Mem_Strdup(Config_Options.config);

    char* dot = strrchr(config, '.');
    if (dot)
      *dot = '\0';

    if  (! Contains_Config(&files, config)) {
      Log_Error("No such configuration available: %s\n", config);
      return NBFC_EXIT_FAILURE;
    }
  }

  // Path =====================================================================
  else {
    config = realpath(Config_Options.config, NULL);

    if (! config) {
      Log_Error("Failed to resolve path '%s': %s\n", Config_Options.config, strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
  }

  ServiceConfig_Load();

  service_config.SelectedConfigId = config;
  ServiceConfig_Set_SelectedConfigId(&service_config);

  Error* e = ServiceConfig_Write(NBFC_SERVICE_CONFIG);
  Mem_Free(config);

  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (Config_Options.action == Config_Action_Apply)
    return Service_Restart(false);

  return NBFC_EXIT_SUCCESS;
}

int Config() {
  switch (Config_Options.action) {
  case Config_Action_List:      return List();
  case Config_Action_Recommend: return Recommend();
  case Config_Action_Set:       return Set_Or_Apply();
  case Config_Action_Apply:     return Set_Or_Apply();
  default:
    printf(CLIENT_CONFIG_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }
}
