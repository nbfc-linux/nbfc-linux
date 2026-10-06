#include <stdio.h> // puts

#include "client_global.h"

#include "../nbfc.h"
#include "../log.h"
#include "../str_functions.h"

const struct cli99_Option ShowVariable_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"variable", Option_ShowVariable_Variable, cli99_NormalPositional},
  cli99_Options_End()
};

struct {
  const char* variable;
} ShowVariable_Options = {0};

int ShowVariable(void) {
  int ret = NBFC_EXIT_SUCCESS;
  const char* const variable = ShowVariable_Options.variable;

  if (! variable) {
    Log_Error("Missing argument: VARIABLE");
    return NBFC_EXIT_CMDLINE;
  }

  if (! str_cmp_ignorecase(variable, "config_file"))
    puts(NBFC_SERVICE_CONFIG);
  else if (! str_cmp_ignorecase(variable, "socket_file"))
    puts(NBFC_SOCKET_PATH);
  else if (! str_cmp_ignorecase(variable, "pid_file"))
    puts(NBFC_PID_FILE);
  else if (! str_cmp_ignorecase(variable, "model_configs_dir"))
    puts(NBFC_MODEL_CONFIGS_DIR);
  else {
    ret = NBFC_EXIT_FAILURE;
    Log_Error("Unknown variable \"%s\". Choose from \"config_file\", \"socket_file\", \"pid_file\", \"model_configs_dir\"",
      variable);
  }

  return ret;
}
