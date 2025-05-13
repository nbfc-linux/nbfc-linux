#ifndef NBFC_FAN_H_
#define NBFC_FAN_H_

#include "macros.h"
#include "error.h"
#include "temperature_threshold_manager.h"
#include "model_config.h"

#include <stdbool.h>

typedef enum {
  Fan_ModeAuto  = 0x0,
  Fan_ModeFixed = 0x1,
} Fan_Mode;

typedef struct Fan Fan;
struct Fan {
  FanConfiguration* fanConfig;        /*const*/
  bool     readWriteWords;            /*const*/
  int      criticalTemperature;       /*const*/
  int      criticalTemperatureOffset; /*const*/
  uint16_t minSpeedValueWrite;        /*const*/
  uint16_t maxSpeedValueWrite;        /*const*/
  uint16_t minSpeedValueRead;         /*const*/
  uint16_t maxSpeedValueRead;         /*const*/
  uint16_t minSpeedValueReadAbs;      /*const*/
  uint16_t maxSpeedValueReadAbs;      /*const*/
  uint16_t fanSpeedSteps;             /*const*/

  ThresholdManager threshMan;
  float targetFanSpeed;
  float requestedSpeed;
  float currentSpeed;
  Fan_Mode mode;
  bool isCritical;
};

Error*   Fan_Init(Fan*, FanConfiguration*, ModelConfig*);

Error*   Fan_UpdateCurrentSpeed(Fan*);
float    Fan_GetCurrentSpeed(const Fan*);
float    Fan_GetTargetSpeed(const Fan*);
float    Fan_GetRequestedSpeed(const Fan*);
uint16_t Fan_GetSpeedSteps(const Fan*);

void     Fan_SetTemperature(Fan*, float temperature);
Error*   Fan_SetFixedSpeed(Fan*, float speed);
void     Fan_SetAutoSpeed(Fan*);

Error*   Fan_ECReset(Fan*);
Error*   Fan_ECFlush(Fan*);

declare_array_of(Fan);

#endif
