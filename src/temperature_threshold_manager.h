#ifndef NBFC_TEMPERATURE_THRESHOLD_MANAGER_H_
#define NBFC_TEMPERATURE_THRESHOLD_MANAGER_H_

#include "model_config.h"
#include "error.h"

#include <stdbool.h>

typedef struct ThresholdManager ThresholdManager;
struct ThresholdManager {
  array_of(TemperatureThreshold) thresholds;
  ssize_t current;
};

Error*                ThresholdManager_Init(ThresholdManager*, array_of(TemperatureThreshold)*);
void                  ThresholdManager_ResetCurrentThreshold(ThresholdManager*, float temperature);
TemperatureThreshold* ThresholdManager_AutoSelectThreshold(ThresholdManager*, float temperature);
TemperatureThreshold* ThresholdManager_GetCurrentThreshold(const ThresholdManager*);

extern bool TemperatureThresholdManager_LegacyBehaviour;

#endif
