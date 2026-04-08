#ifndef NBFC_SERVICE_CONTROL_H_
#define NBFC_SERVICE_CONTROL_H_

#include <stdbool.h>
#include <sys/types.h> // pid_t

#include "../error.h"
#include "../nxjson.h"
#include "../model_config.h"

pid_t Service_Get_PID();
int   Service_Start(bool);
int   Service_Stop();
int   Service_Restart(bool);

Error Client_Communicate(const nx_json*, char**, const nx_json**);
Error ServiceInfo_TryLoad(ServiceInfo*);
void  ServiceConfig_Load();
void  Service_LoadAllConfigFiles(ModelConfig*);

#endif
