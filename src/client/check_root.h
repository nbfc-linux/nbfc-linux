#ifndef CHECK_ROOT_H_
#define CHECK_ROOT_H_

#include <stdlib.h> // exit
#include <unistd.h> // geteuid

#include "../nbfc.h"
#include "../log.h"

static inline void check_root() {
  if (geteuid()) {
    Log_Error("This operation must be run as root\n");
    exit(NBFC_EXIT_FAILURE);
  }
}

#endif
