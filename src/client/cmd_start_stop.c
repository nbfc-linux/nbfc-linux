#include "check_root.h"
#include "service_control.h"
#include "client_global.h"

const struct cli99_Option start_options[] = {
  cli99_Options_Include(&main_options),
  {"-r|--read-only", Option_Start_ReadOnly, cli99_NoArgument},
  cli99_Options_End()
};

struct {
  bool read_only;
} Start_Options = {0};

int Start() {
  check_root();
  return Service_Start(Start_Options.read_only);
}

int Stop() {
  check_root();
  return Service_Stop();
}

int Restart() {
  check_root();
  return Service_Restart(Start_Options.read_only);
}
