#ifndef NBFC_SERVICE_H_
#define NBFC_SERVICE_H_

#include "config.h"
#include "error.h"
#include "fan.h"
#include "fan_temperature_control.h"
#include "model_config.h"
#include "temperature_filter.h"

#include <stdbool.h>
#include <pthread.h>

typedef struct Service_Options Service_Options;
struct Service_Options {
  EmbeddedControllerType embedded_controller_type;
  bool                   fork;
  bool                   read_only;
  bool                   debug;
  bool                   python_hack;
  const char*            service_config;
};

extern ModelConfig     Service_Model_Config;
extern pthread_mutex_t Service_Lock;
extern array_of(FanTemperatureControl) Service_Fans;
extern Service_Options options;

Error* Service_Init();
Error* Service_Loop();
void   Service_Cleanup();
Error* Service_WriteTargetFanSpeedsToConfig();

#endif
