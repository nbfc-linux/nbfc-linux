#ifndef NBFC_LM_SENSORS_H_
#define NBFC_LM_SENSORS_H_

#include "sensors.h"

Error* LM_Sensors_Init();
void   LM_Sensors_Cleanup();
Error* LM_Sensors_GetTemperature(float*);

extern Sensor_VTable LM_Sensors_VTable;

#endif
