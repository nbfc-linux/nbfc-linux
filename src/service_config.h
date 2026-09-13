#ifndef NBFC_SERVICE_CONFIG_H_
#define NBFC_SERVICE_CONFIG_H_

#include "model_config.h"

Error ServiceConfig_FromFile(ServiceConfig*, const char*);
Error ServiceConfig_Write(const ServiceConfig*, const char*);
void  ServiceConfig_Free(ServiceConfig*);

#endif
