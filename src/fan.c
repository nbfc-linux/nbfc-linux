#include "fan.h"

#include "error.h"
#include "ec.h"

#include <math.h>

extern EC_VTable* ec;

Error* Fan_Init(Fan* self, FanConfiguration* cfg, int criticalTemperature, bool readWriteWords) {
  my.fanConfig            = cfg;
  my.mode                 = Fan_ModeAuto;
  my.criticalTemperature  = criticalTemperature;
  my.readWriteWords       = readWriteWords;
  my.minSpeedValueWrite   = cfg->MinSpeedValue;
  my.maxSpeedValueWrite   = cfg->MaxSpeedValue;
  my.criticalTemperatureOffset = 15;
  const int same = ! cfg->IndependentReadMinMaxValues;
  my.minSpeedValueRead    = same ? my.minSpeedValueWrite : cfg->MinSpeedValueRead;
  my.maxSpeedValueRead    = same ? my.maxSpeedValueWrite : cfg->MaxSpeedValueRead;
  my.minSpeedValueReadAbs = min(my.minSpeedValueRead, my.maxSpeedValueRead);
  my.maxSpeedValueReadAbs = max(my.minSpeedValueRead, my.maxSpeedValueRead);
  my.fanSpeedSteps        = my.maxSpeedValueReadAbs - my.minSpeedValueReadAbs;

  // TODO #1: How to handle empty TemperatureThresholds? [see model_config.c]
  if (! cfg->TemperatureThresholds.size)
    return err_string(0, "Invalid size for cfg->TemperatureThresholds");

  return ThresholdManager_Init(&my.threshMan, &cfg->TemperatureThresholds);
}

// ============================================================================
// PRIVATE
// ============================================================================

static inline int float_eq(const float a, const float b) {
  return fabs(a - b) < 0.06; /* ~ 0.05 */
}

static FanSpeedPercentageOverride* Fan_OverrideByValue(const Fan* self, int value) {
  for_each_array(FanSpeedPercentageOverride*, o, my.fanConfig->FanSpeedPercentageOverrides)
    if ((o->TargetOperation & OverrideTargetOperation_Read) &&
         o->FanSpeedValue == value)
      return o;

  return NULL;
}

static FanSpeedPercentageOverride* Fan_OverrideByPercentage(const Fan* self, float percentage) {
  for_each_array(FanSpeedPercentageOverride*, o, my.fanConfig->FanSpeedPercentageOverrides)
    if ((o->TargetOperation & OverrideTargetOperation_Write) &&
        float_eq(o->FanSpeedPercentage, percentage))
      return o;

  return NULL;
}

static int Fan_PercentageToFanSpeed(const Fan* self, float percentage) {
  if (percentage > 100.0f)
    percentage = 100.0f;
  else if (percentage < 0.0f)
    percentage = 0.0f;

  FanSpeedPercentageOverride* override = Fan_OverrideByPercentage(self, percentage);
  if (override)
    return override->FanSpeedValue;

  return round(my.minSpeedValueWrite
      + (((my.maxSpeedValueWrite - my.minSpeedValueWrite) * percentage) / 100.0f));
}

static float Fan_FanSpeedToPercentage(const Fan* self, int fanSpeed) {
  FanSpeedPercentageOverride* override = Fan_OverrideByValue(self, fanSpeed);
  if (override)
    return override->FanSpeedPercentage;

  if (my.minSpeedValueRead == my.maxSpeedValueRead)
    return 0.0f; /* division by zero */

  return ((float)(fanSpeed - my.minSpeedValueRead) /
     (my.maxSpeedValueRead - my.minSpeedValueRead)) * 100.0f;
}

static Error* Fan_ECWriteValue(Fan* self, int value) {
  return my.readWriteWords
    ? ec->WriteWord(my.fanConfig->WriteRegister, value)
    : ec->WriteByte(my.fanConfig->WriteRegister, value);
}

static Error* Fan_ECReadValue(const Fan* self, int* out) {
  if (my.readWriteWords) {
    uint16_t word;
    Error* e = ec->ReadWord(my.fanConfig->ReadRegister, &word);
    if (!e)
      *out = word;
    return e;
  }
  else {
    uint8_t byte;
    Error* e = ec->ReadByte(my.fanConfig->ReadRegister, &byte);
    if (!e)
      *out = byte;
    return e;
  }
}

// ============================================================================
// PUBLIC
// ============================================================================

float Fan_GetTargetSpeed(const Fan* self) {
  return my.isCritical ? 100.0f : my.targetFanSpeed;
}

void Fan_SetTemperature(Fan* self, float temperature)
{
  // HandleCritalMode
  if (temperature > my.criticalTemperature)
    my.isCritical = true;
  else if (temperature < (my.criticalTemperature - my.criticalTemperatureOffset))
    my.isCritical = false;

  TemperatureThreshold* threshold = ThresholdManager_AutoSelectThreshold(&my.threshMan, temperature);
  if (my.mode == Fan_ModeAuto && threshold)
    my.targetFanSpeed = threshold->FanSpeed;
}

Error* Fan_SetFixedSpeed(Fan* self, float speed) {
  my.mode = Fan_ModeFixed;

  if (speed < 0.0f) {
    my.targetFanSpeed = 0.0f;
    return err_string(0, "speed < 0.0");
  }
  else if (speed > 100.0f) {
    my.targetFanSpeed = 100.0f;
    return err_string(0, "speed > 100.0");
  }

  my.targetFanSpeed = speed;
  return err_success();
}

void Fan_SetAutoSpeed(Fan* self) {
  my.mode = Fan_ModeAuto;
  if (my.threshMan.current)
    my.targetFanSpeed = my.threshMan.current->FanSpeed;
}

float Fan_GetCurrentSpeed(const Fan* self) {
  return my.currentSpeed;
}

Error* Fan_UpdateCurrentSpeed(Fan* self) {
  int speed;

  // If the value is out of range 3 or more times,
  // minFanSpeed and/or maxFanSpeed are probably wrong.
  for (range(int, i, 0, 3)) {
    Error* e = Fan_ECReadValue(self, &speed);
    if (e)
      return e;

    if (speed >= my.minSpeedValueReadAbs && speed <= my.maxSpeedValueReadAbs) {
      break;
    }
  }

  my.currentSpeed = Fan_FanSpeedToPercentage(self, speed);
  return err_success();
}

int Fan_GetSpeedSteps(const Fan* self) {
  return my.fanSpeedSteps;
}

Error* Fan_ECReset(Fan* self) {
  if (my.fanConfig->ResetRequired)
    return Fan_ECWriteValue(self, my.fanConfig->FanSpeedResetValue);
  return err_success();
}

Error* Fan_ECFlush(Fan* self) {
  const float speed = Fan_GetTargetSpeed(self);
  const int   value = Fan_PercentageToFanSpeed(self, speed);
  return Fan_ECWriteValue(self, value);
}

