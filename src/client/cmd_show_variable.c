#include <stdio.h>  // printf
#include <string.h> // strcmp

#include "str_functions.h"
#include "client_global.h"

#include "../nbfc.h"
#include "../memory.h"
#include "../log.h"

const cli99_option show_variable_options[] = {
  cli99_include_options(&main_options),
  {"variable", Option_ShowVariable_Variable, 1},
  cli99_options_end()
};

struct {
  const char* variable;
} Show_Variable_Options = {0};

int Show_Variable() {
  if (! Show_Variable_Options.variable) {
    Log_Error("Missing argument: VARIABLE\n");
    return NBFC_EXIT_CMDLINE;
  }

  int ret = NBFC_EXIT_SUCCESS;
  char* variable = str_to_lower(Show_Variable_Options.variable);

  if (! strcmp(variable, "config_file"))
    printf("%s\n", NBFC_SERVICE_CONFIG);
  else if (! strcmp(variable, "socket_file"))
    printf("%s\n", NBFC_SOCKET_PATH);
  else if (! strcmp(variable, "pid_file"))
    printf("%s\n", NBFC_PID_FILE);
  else if (! strcmp(variable, "model_configs_dir"))
    printf("%s\n", NBFC_MODEL_CONFIGS_DIR);
  else {
    ret = NBFC_EXIT_FAILURE;
    Log_Error("Unknown variable '%s'. Choose from 'config_file', 'socket_file', 'pid_file', 'model_configs_dir'\n",
      Show_Variable_Options.variable);
  }

  Mem_Free(variable);
  return ret;
}
