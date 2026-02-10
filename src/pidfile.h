#ifndef NBFC_PIDFILE_H_
#define NBFC_PIDFILE_H_

#include "error.h"

#include <fcntl.h>

enum PID_LockMode {
  PID_AcquireLock = O_EXCL,
  PID_NoAcquireLock = 0
};

Error PID_Write(enum PID_LockMode);
void  PID_Cleanup();

#endif
