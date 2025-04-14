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
  array_of(ConfigFile) files = get_configs();
  qsort(files.data, files.size, sizeof(struct ConfigFile), compare_config_by_name);
  for_each_array(ConfigFile*, file, files) {
    printf("%s\n", file->config_name);
  }

  return NBFC_EXIT_SUCCESS;
}

int Recommend() {
  const char* model_name = get_model_name();
  array_of(ConfigFile) files = recommended_configs();
  char* matched_model = get_supported_model(&files, model_name);

  if (matched_model) {
    fprintf(stderr, "Found supported model:\n");
    printf("%s\n", matched_model);
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
  char *model;
  char path[PATH_MAX];

  if (!strcmp(Config_Options.config, "auto")) {
    const char* model_name = get_model_name();
    array_of(ConfigFile) files = recommended_configs();
    model = get_supported_model(&files, model_name);
    if (! model) {
      Log_Error("No config found to apply automatically\n");
      return NBFC_EXIT_FAILURE;
    }
  }
  else {
    if (strrchr(Config_Options.config, '/')) {
      if (! realpath(Config_Options.config, path)) {
        Log_Error("Failed to resolve path '%s': %s\n", Config_Options.config, strerror(errno));
        return NBFC_EXIT_FAILURE;
      }

      char* slash = strrchr(path, '/');
      *slash = '\0';
      model = slash + 1;

      if (strcmp(path, NBFC_MODEL_CONFIGS_DIR)) {
        Log_Error("File does not reside in model configs dir (%s): %s\n",
          NBFC_MODEL_CONFIGS_DIR, Config_Options.config);
        return NBFC_EXIT_FAILURE;
      }
    }
    else {
      snprintf(path, sizeof(path), "%s", Config_Options.config);
      model = path;
    }

    char* dot = strrchr(model, '.');
    if (dot)
      *dot = '\0';

    char file[PATH_MAX];
    snprintf(file, sizeof(file), NBFC_MODEL_CONFIGS_DIR "/%s.json", model);
    if (access(file, F_OK)) {
      Log_Error("No such config file found: %s\n", file);
      return NBFC_EXIT_FAILURE;
    }
  }

  ServiceConfig_Load();
  service_config.SelectedConfigId = model;
  Error* e = ServiceConfig_Write(NBFC_SERVICE_CONFIG);
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
