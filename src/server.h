#ifndef SERVER_H_
#define SERVER_H_

#include "error.h"

Error* Server_Init();
void   Server_Close();
Error* Server_Start();
void   Server_Stop();

#endif
