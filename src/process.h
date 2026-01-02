#ifndef NBFC_PROCESS_H
#define NBFC_PROCESS_H

#include "error.h"

#include <stdlib.h>

static inline Error Process_Call(const char* cmd) {
  switch (system(cmd)) {
  case 0:  return err_success();
  case -1: return err_stdlib("system()");
  default: return err_stringf("Could not execute `%s'", cmd);
  }
}

#endif
