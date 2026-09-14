#ifndef NBFC_FS_SENSORS_H_
#define NBFC_FS_SENSORS_H_

#include "error.h"
#include "macros.h"
#include "model_config.h"

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

extern array_of(FS_TemperatureSource) FS_Sensors_Sources;

typedef array_of(array_size_t) FS_TemperatureSource_References;

static inline array_size_t FS_Sensors_Sources_Ref(FS_TemperatureSource* pointer) {
  return pointer - FS_Sensors_Sources.data;
}

static inline FS_TemperatureSource* FS_Sensors_Sources_UnRef(size_t idx) {
  return &FS_Sensors_Sources.data[idx];
}

Error FS_Sensors_Init(void);
void  FS_Sensors_Cleanup(void);
void  FS_Sensors_Log(void);
Error FS_TemperatureSource_GetTemperature(const FS_TemperatureSource*, float*);
Error FS_TemperatureSources_GetTemperature(FS_TemperatureSource_References*, TemperatureAlgorithmType, float*);
Error FS_TemperatureSources_AddTemperatureSources(FS_TemperatureSource_References*, const char*);

#endif
