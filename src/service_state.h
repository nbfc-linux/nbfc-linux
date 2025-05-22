#ifndef NBFC_SERVICE_STATE_H_
#define NBFC_SERVICE_STATE_H_

#include "model_config.h"

extern ServiceState service_state;

Error* ServiceState_Init();
Error* ServiceState_Write();
void   ServiceState_Free();

#endif
