#ifndef NBFC_FS_SENSORS_H_
#define NBFC_FS_SENSORS_H_

#include "sensors.h"

Error* FS_Sensors_Init();
void   FS_Sensors_Cleanup();
Error* FS_Sensors_GetTemperature(float*);

extern Sensor_VTable FS_Sensors_VTable;

#endif
