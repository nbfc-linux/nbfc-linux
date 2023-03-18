#ifndef NBFC_INFO_H_
#define NBFC_INFO_H_

#include "error.h"
#include "fan.h"

Error* Info_Init(const char*);
void   Info_Close();
Error* Info_Write(ModelConfig*, float temperature, bool readonly, array_of(Fan)*);

#endif
