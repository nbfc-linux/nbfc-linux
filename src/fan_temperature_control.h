#ifndef FAN_TEMPERATURE_CONTROL_H_
#define FAN_TEMPERATURE_CONTROL_H_

#include "fan.h"
#include "fs_sensors.h"
#include "model_config.h"
#include "temperature_filter.h"

#define FAN_TEMPERATURE_CONTROL_MAX_SOURCES 32

struct FanTemperatureControl {
  Fan                      Fan;
  FS_TemperatureSource*    TemperatureSources[FAN_TEMPERATURE_CONTROL_MAX_SOURCES];
  int                      TemperatureSourcesSize;
  TemperatureAlgorithmType TemperatureAlgorithmType;
  TemperatureFilter        TemperatureFilter;
  float                    Temperature;
};
typedef struct FanTemperatureControl FanTemperatureControl;
declare_array_of(FanTemperatureControl);

Error* FanTemperatureControl_Init(array_of(FanTemperatureControl)*, ServiceConfig*, ModelConfig*);
Error* FanTemperatureControl_UpdateFanTemperature(FanTemperatureControl*);
void   FanTemperatureControl_Log(array_of(FanTemperatureControl)*, ModelConfig*);

#endif
