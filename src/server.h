#ifndef SERVER_H_
#define SERVER_H_

#include "error.h"

Error* Server_Init();
Error* Server_Loop(int);
void   Server_Close();

#endif
