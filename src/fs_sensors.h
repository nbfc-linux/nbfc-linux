#ifndef NBFC_FS_SENSORS_H_
#define NBFC_FS_SENSORS_H_

#include "error.h"
#include "macros.h"

enum FS_TemperatureSource_Type {
  FS_TemperatureSource_File,
  FS_TemperatureSource_Command,
  FS_TemperatureSource_Nvidia,
};
typedef enum FS_TemperatureSource_Type FS_TemperatureSource_Type;

struct FS_TemperatureSource {
  char* name;
  char* file;
  float multiplier;
  FS_TemperatureSource_Type type;
};
typedef struct FS_TemperatureSource FS_TemperatureSource;
declare_array_of(FS_TemperatureSource);

Error* FS_Sensors_Init();
void   FS_Sensors_Cleanup();
void   FS_Sensors_Log();
Error* FS_TemperatureSource_GetTemperature(FS_TemperatureSource*, float*);

extern array_of(FS_TemperatureSource) FS_Sensors_Sources;

#endif
