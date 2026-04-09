#include <stdio.h>  // printf
#include <string.h> // strcmp

#include "str_functions.h"
#include "client_global.h"

#include "../nbfc.h"
#include "../memory.h"
#include "../log.h"

const struct cli99_Option show_variable_options[] = {
  cli99_Options_Include(&main_options),
  {"variable", Option_ShowVariable_Variable, cli99_RequiredArgument},
  cli99_Options_End()
};

struct {
  const char* variable;
} Show_Variable_Options = {0};

int Show_Variable(void) {
  int ret = NBFC_EXIT_SUCCESS;
  const char* const variable = Show_Variable_Options.variable;

  if (! variable) {
    Log_Error("Missing argument: VARIABLE");
    return NBFC_EXIT_CMDLINE;
  }

  if (! str_cmp_ignorecase(variable, "config_file"))
    printf("%s\n", NBFC_SERVICE_CONFIG);
  else if (! str_cmp_ignorecase(variable, "socket_file"))
    printf("%s\n", NBFC_SOCKET_PATH);
  else if (! str_cmp_ignorecase(variable, "pid_file"))
    printf("%s\n", NBFC_PID_FILE);
  else if (! str_cmp_ignorecase(variable, "model_configs_dir"))
    printf("%s\n", NBFC_MODEL_CONFIGS_DIR);
  else {
    ret = NBFC_EXIT_FAILURE;
    Log_Error("Unknown variable \"%s\". Choose from \"config_file\", \"socket_file\", \"pid_file\", \"model_configs_dir\"",
      variable);
  }

  return ret;
}
