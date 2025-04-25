#ifndef SERVER_H_
#define SERVER_H_

#include "error.h"

#include <sys/time.h>

Error* Server_Init();
Error* Server_Loop(struct timeval*);
void   Server_Close();

#endif
