#ifndef PIDFILE_H
#define PIDFILE_H

#include "error.h"

Error* PID_Write(bool);
void   PID_Cleanup();

#endif
