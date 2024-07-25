#ifndef NBFC_FS_SENSORS_H_
#define NBFC_FS_SENSORS_H_

#include "error.h"
#include "macros.h"

struct FS_TemperatureSource {
  char* name;
  char* file;
  float multiplier;
};
typedef struct FS_TemperatureSource FS_TemperatureSource;
declare_array_of(FS_TemperatureSource);

Error* FS_Sensors_Init();
void   FS_Sensors_Cleanup();
Error* FS_TemperatureSource_GetTemperature(FS_TemperatureSource*, float*);

extern array_of(FS_TemperatureSource) FS_Sensors_Sources;

#endif
