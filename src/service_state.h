#ifndef NBFC_SERVICE_STATE_H_
#define NBFC_SERVICE_STATE_H_

#include "model_config.h"

Error ServiceState_FromFile(ServiceState*, const char*);
Error ServiceState_Write(const ServiceState*, const char*);
void  ServiceState_Free(ServiceState*);

#endif
