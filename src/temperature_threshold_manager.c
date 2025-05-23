#include "temperature_threshold_manager.h"

bool TemperatureThresholdManager_LegacyBehaviour = false;

Error* ThresholdManager_Init(ThresholdManager* self, array_of(TemperatureThreshold)* thresholds) {
  if (! thresholds->size)
    return err_string(0, "Invalid size for TemperatureThresholds");

  my.current = 0;
  my.thresholds = *thresholds;

  /* Bubble sort - ascending */
  for (ssize_t i = 0; i < thresholds->size - 1; ++i)
    for (ssize_t j = 0; j < thresholds->size - i - 1; ++j) {
      TemperatureThreshold* a = &thresholds->data[j];
      TemperatureThreshold* b = &thresholds->data[j+1];
      if (a->UpThreshold > b->UpThreshold) {
        const TemperatureThreshold swap = *a;
        *a = *b;
        *b = swap;
      }
    }

  return err_success();
}

TemperatureThreshold* ThresholdManager_AutoSelectThreshold(ThresholdManager* self, float temperature) {
  const ssize_t size = my.thresholds.size;
  size_t i = my.current;

  if (TemperatureThresholdManager_LegacyBehaviour) {
    while (i > 0        && temperature <= my.thresholds.data[i].DownThreshold)   --i;
    while (i < size - 1 && temperature >= my.thresholds.data[i + 1].UpThreshold) ++i;
  }
  else {
    while (i > 0        && temperature <= my.thresholds.data[i].DownThreshold) --i;
    while (i < size - 1 && temperature >= my.thresholds.data[i].UpThreshold)   ++i;
  }

  my.current = i;

  return &my.thresholds.data[my.current];
}

TemperatureThreshold* ThresholdManager_GetCurrentThreshold(const ThresholdManager* self) {
  return &my.thresholds.data[my.current];
}
