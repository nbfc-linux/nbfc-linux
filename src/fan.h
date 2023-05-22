#ifndef NBFC_FAN_H_
#define NBFC_FAN_H_

#include "macros.h"
#include "error.h"
#include "temperature_threshold_manager.h"

#include <stdbool.h>

typedef enum {
  Fan_ModeAuto  = 0x0,
  Fan_ModeFixed = 0x1,
} Fan_Mode;

typedef struct Fan Fan;
struct Fan {
  FanConfiguration* fanConfig;     /*const*/
  bool  readWriteWords;            /*const*/
  int   criticalTemperature;       /*const*/
  int   minSpeedValueWrite;        /*const*/
  int   maxSpeedValueWrite;        /*const*/
  int   minSpeedValueRead;         /*const*/
  int   maxSpeedValueRead;         /*const*/
  int   minSpeedValueReadAbs;      /*const*/
  int   maxSpeedValueReadAbs;      /*const*/
  int   fanSpeedSteps;             /*const*/
  int   criticalTemperatureOffset;

  ThresholdManager threshMan;
  float targetFanSpeed;
  float currentSpeed;
  Fan_Mode mode;
  bool isCritical;
};

Error* Fan_Init(Fan*, FanConfiguration*, int criticalTemperature, bool readWriteWords);

Error* Fan_UpdateCurrentSpeed(Fan*);
float  Fan_GetCurrentSpeed(const Fan*);
float  Fan_GetTargetSpeed(const Fan*);
int    Fan_GetSpeedSteps(const Fan*);

void   Fan_SetTemperature(Fan*, float temperature);
Error* Fan_SetFixedSpeed(Fan*, float speed);
void   Fan_SetAutoSpeed(Fan*);

Error* Fan_ECReset(Fan*);
Error* Fan_ECFlush(Fan*);

declare_array_of(Fan);

#endif
