#ifndef NBFC_PROCESS_H_
#define NBFC_PROCESS_H_

#include "error.h"

#include <stdlib.h> // system

static inline Error Process_Call(const char* cmd) {
  int ret = system(cmd);
  switch (ret) {
  case 0:  return err_success();
  case -1: return err_stdlib(cmd);
  default: return err_stringf("Command `%s' returned %d", cmd, WEXITSTATUS(ret));
  }
}

int Process_Capture(const char* file, char* const argv[], char** out_stdout, char** out_stderr);

#endif
