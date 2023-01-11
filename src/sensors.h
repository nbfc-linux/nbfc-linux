#ifndef NBFC_SENSORS_H
#define NBFC_SENSORS_H

#include "error.h"

typedef struct Sensor_VTable Sensor_VTable;
struct Sensor_VTable {
  Error*   (*Init)();
  void     (*Cleanup)();
  Error*   (*GetTemperature)(float*);
};

#endif
