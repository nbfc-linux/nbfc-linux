#ifndef NBFC_SERVICE_H_
#define NBFC_SERVICE_H_

#include "error.h"
#include "fan.h"
#include "model_config.h"

#include <stdbool.h>
#include <pthread.h>

typedef struct Service_Options Service_Options;
struct Service_Options {
  EmbeddedControllerType embedded_controller_type;
  bool                   fork;
  bool                   read_only;
  bool                   debug;
  const char*            service_config;
};

extern ModelConfig     Service_Model_Config;
extern array_of(Fan)   Service_Fans;
extern float           Service_Temperature;
extern pthread_mutex_t Service_Lock;
extern Service_Options options;

Error* Service_Init();
void   Service_Loop();
void   Service_Cleanup();
void   Service_UpdateFanSpeedsByTargetFanSpeeds();

#endif
