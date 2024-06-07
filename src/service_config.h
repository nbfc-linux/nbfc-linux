#ifndef NBFC_SERVICE_CONFIG_H_
#define NBFC_SERVICE_CONFIG_H_

#include "model_config.h"

extern ServiceConfig service_config;

Error* ServiceConfig_Init(const char*);
void   ServiceConfig_Free(ServiceConfig*);
Error* ServiceConfig_Write(const char*);

#endif
