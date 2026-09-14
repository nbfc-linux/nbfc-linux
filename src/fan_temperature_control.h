#ifndef NBFC_FAN_TEMPERATURE_CONTROL_H_
#define NBFC_FAN_TEMPERATURE_CONTROL_H_

#include "fan.h"
#include "fs_sensors.h"
#include "model_config.h"
#include "temperature_filter.h"

struct FanTemperatureControl {
  Fan                      Fan;
  FS_TemperatureSource_References TemperatureSources;
  TemperatureAlgorithmType TemperatureAlgorithmType;
  TemperatureFilter        TemperatureFilter;
  float                    Temperature;
};
typedef struct FanTemperatureControl FanTemperatureControl;
declare_array_of(FanTemperatureControl);

Error FanTemperatureControls_Init(array_of(FanTemperatureControl)*, ServiceConfig*, ModelConfig*);
void  FanTemperatureControls_Free(array_of(FanTemperatureControl)*);
void  FanTemperatureControls_Log(const array_of(FanTemperatureControl)*, ModelConfig*);
Error FanTemperatureControl_UpdateFanTemperature(FanTemperatureControl*);

#endif
