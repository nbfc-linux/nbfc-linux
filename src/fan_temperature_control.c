#include "fan_temperature_control.h"

#include "nbfc.h"

#include <float.h>

static const char* const LinuxTempSensorNames[] = {
  "coretemp", "k10temp", "zenpower"
};

static inline int IsLinuxTempSensorName(const char* s) {
  for (int i = 0; i < ARRAY_SSIZE(LinuxTempSensorNames); ++i)
    if (! strcmp(s, LinuxTempSensorNames[i]))
      return true;
  return false;
}

static Error* FanTemperatureControl_GetTemperature(FanTemperatureControl* ftc, float* out) {
  float tmp;
  float sum = 0;
  float min = FLT_MAX;
  float max = FLT_MIN;
  int   total = 0;

  for (int i = 0; i < ftc->TemperatureSourcesSize; ++i) {
    FS_TemperatureSource* ts = ftc->TemperatureSources[i];
    Error* e = FS_TemperatureSource_GetTemperature(ts, &tmp);
    if (! e) {
      min = min(min, tmp);
      max = max(max, tmp);
      sum += tmp;
      total++;
    }
  }

  if (! total)
    return err_string(0, "No temperatures available");

  switch (ftc->TemperatureAlgorithmType) {
    case TemperatureAlgorithmType_Average:
      *out = sum / total;
      return err_success();
    case TemperatureAlgorithmType_Min:
      *out = min;
      return err_success();
    case TemperatureAlgorithmType_Max:
      *out = max;
      return err_success();
    default:
      return err_string(0, "FanTemperatureControl_GetTemperature: Invalid value for type");
  }
}

Error* FanTemperatureControl_SetDefaults(FanTemperatureControl* ftc, int poll_interval) {
  Error* e;
  ftc->TemperatureAlgorithmType = TemperatureAlgorithmType_Average;

  for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
    if (IsLinuxTempSensorName(ts->name)) {
      if (ftc->TemperatureSourcesSize >= FAN_TEMPERATURE_CONTROL_MAX_SOURCES) {
        return err_string(0, "Too many temperature sources found");
      }

      ftc->TemperatureSources[ftc->TemperatureSourcesSize++] = ts;
    }
  }

  e = TemperatureFilter_Init(&ftc->TemperatureFilter, poll_interval, NBFC_TEMPERATURE_FILTER_TIMESPAN);
  return e;
}

Error* FanTemperatureControl_SetByConfig(
  array_of(FanTemperatureControl)* fans ,
  array_of(FanTemperatureSourceConfig)* FanTemperatureSources,
  ModelConfig* model_config)
{
  Error* e;

  for_each_array(FanTemperatureSourceConfig*, ftsc, *FanTemperatureSources) {
    if (ftsc->FanIndex < 0 || ftsc->FanIndex >= model_config->FanConfigurations.size)
      return err_string(0, "Invalid FanIndex in FanTemperatureSources");

    FanTemperatureControl* ftc = &fans->data[ftsc->FanIndex];
    ftc->TemperatureAlgorithmType = ftsc->TemperatureAlgorithmType;

    // If no sensors are given, use the defaults
    if (! ftsc->Sensors.size)
      continue;

    ftc->TemperatureSourcesSize = 0;
    for_each_array(const char**, sensor, ftsc->Sensors) {
      bool found_sensor = false;

      for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
        if (!strcmp(*sensor, ts->name) || !strcmp(*sensor, ts->file)) {
          if (ftc->TemperatureSourcesSize >= FAN_TEMPERATURE_CONTROL_MAX_SOURCES)
            return err_string(0, "Too many temperature sources found");
          ftc->TemperatureSources[ftc->TemperatureSourcesSize++] = ts;
          found_sensor = true;
        }
      }

      if (! found_sensor) {
        e = err_string(0, "Sensor not found");
        e = err_string(e, *sensor);
        return e;
      }
    }
  }

  return err_success();
}

Error* FanTemperatureControl_UpdateFanTemperature(FanTemperatureControl* ftc) {
  float temp;
  Error* e = FanTemperatureControl_GetTemperature(ftc, &temp);
  if (e)
    return e;

  ftc->Temperature = TemperatureFilter_FilterTemperature(&ftc->TemperatureFilter, temp);
  return err_success();
}

void FanTemperatureControl_Log(array_of(FanTemperatureControl)* fans, ModelConfig* model_config) {
  for_enumerate_array(int, fan_index, *fans) {
    FanTemperatureControl* ftc = &fans->data[fan_index];

    for (int i = 0; i < ftc->TemperatureSourcesSize; ++i)
      Log_Info("Fan #%d (%s) uses '%s' (%s) as temperature source\n",
        fan_index,
        model_config->FanConfigurations.data[fan_index].FanDisplayName,
        ftc->TemperatureSources[i]->name,
        ftc->TemperatureSources[i]->file);
  }
}

