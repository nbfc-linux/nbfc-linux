#include "check_root.h"
#include "service_control.h"
#include "client_global.h"

const cli99_option start_options[] = {
  cli99_include_options(&main_options),
  {"-r|--read-only", Option_Start_ReadOnly, 0},
  cli99_options_end()
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
