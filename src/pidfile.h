#ifndef PIDFILE_H
#define PIDFILE_H

#include "error.h"

#include <stdbool.h>

Error* PID_Write(bool);
void   PID_Cleanup();

#endif
