#include "fan_temperature_control.h"

#include "nbfc.h"
#include "memory.h"

#include <float.h>
#include <string.h>

static const char* const CPUSensorNames[] = {
  "coretemp", "k10temp", "zenpower"
};

static const char* const GPUSensorNames[] = {
  "amdgpu", "nvidia", "nvidia-ml", "nouveau", "radeon"
};

static inline int IsCPUSensorName(const char* s) {
  for (int i = 0; i < ARRAY_SSIZE(CPUSensorNames); ++i)
    if (! strcmp(s, CPUSensorNames[i]))
      return true;
  return false;
}

static inline int IsGPUSensorName(const char* s) {
  for (int i = 0; i < ARRAY_SSIZE(GPUSensorNames); ++i)
    if (! strcmp(s, GPUSensorNames[i]))
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
    e_warn();
    if (! e) {
      min = min(min, tmp);
      max = max(max, tmp);
      sum += tmp;
      ++total;
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

// Add a TemperatureSource to a FanTemperatureControl.
// Return error if maxiumum size of TemperatureSources is exceeded.
static Error* FanTemperatureControl_AddTemperatureSource(
  FanTemperatureControl* ftc,
  FS_TemperatureSource* ts)
{
  if (ftc->TemperatureSourcesSize >= FAN_TEMPERATURE_CONTROL_MAX_SOURCES)
    return err_string(0, "Too many temperature sources found");

  ftc->TemperatureSources[ftc->TemperatureSourcesSize++] = ts;
  return err_success();
}

// Adds one or more TemperatureSources to a FanTemperatureControl.
//
// If `sensor` is not found in `FS_Sensors_Sources` by its name or its path,
// this function assumes that `sensor` is a user defined file path to a file
// containing the temperature.
//
// Return error if `sensor` is not found in available temperature sources
// or `sensor` is not a valid file path to a temperature file.
static Error* FanTemperatureControl_AddTemperatureSources(
  FanTemperatureControl* ftc,
  const char* sensor)
{
  Error* e;
  bool found_sensors = false;

  // ==========================================================================
  // Sensor group "@CPU": Add all sensors found in `CPUSensorNames`
  // ==========================================================================
  if (!strcmp(sensor, "@CPU")) {
    for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
      if (IsCPUSensorName(ts->name)) {
        e = FanTemperatureControl_AddTemperatureSource(ftc, ts);
        if (e)
          return e;

        found_sensors = true;
      }
    }

    return found_sensors
      ? err_success()
      : err_stringf(0, "%s: No sensors found", "@CPU");
  }

  // ==========================================================================
  // Sensor group "@GPU": Add all sensors found in `GPUSensorNames`
  // ==========================================================================
  if (!strcmp(sensor, "@GPU")) {
    for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
      if (IsGPUSensorName(ts->name)) {
        e = FanTemperatureControl_AddTemperatureSource(ftc, ts);
        if (e)
          return e;

        found_sensors = true;
      }
    }

    return found_sensors
      ? err_success()
      : err_stringf(0, "%s: No sensors found", "@GPU");
  }

  // ==========================================================================
  // Add sensors by name or path (for available sensors)
  // ==========================================================================
  for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
    if (!strcmp(sensor, ts->name) || !strcmp(sensor, ts->file)) {
      e = FanTemperatureControl_AddTemperatureSource(ftc, ts);
      if (e)
        return e;

      found_sensors = true;
    }
  }

  if (found_sensors)
    return err_success();

  // ==========================================================================
  // Create a new TemperatureSource (a user defined file or command)
  // ==========================================================================
  FS_TemperatureSource source;

  if (sensor[0] == '$') {
    // Sensor is a command
    source.name = "command";
    source.file = (char*) sensor + 1;
    source.type = FS_TemperatureSource_Command;
    source.multiplier = 1;
  }
  else {
    // Sensor is a user defined file
    source.name = "anonymous";
    source.file = (char*) sensor;
    source.type = FS_TemperatureSource_File;
    source.multiplier = 0.001;
  }

  float t; // NOLINT
  e = FS_TemperatureSource_GetTemperature(&source, &t);
  if (e)
    return e;

  const size_t idx = FS_Sensors_Sources.size;
  FS_Sensors_Sources.data = Mem_Realloc(FS_Sensors_Sources.data, (idx + 1) * sizeof(FS_TemperatureSource));
  FS_Sensors_Sources.data[idx].name = Mem_Strdup(source.name);
  FS_Sensors_Sources.data[idx].file = Mem_Strdup(source.file);
  FS_Sensors_Sources.data[idx].multiplier = source.multiplier;
  FS_Sensors_Sources.data[idx].type = source.type;
  FS_Sensors_Sources.size = idx + 1;

  e = FanTemperatureControl_AddTemperatureSource(ftc, &FS_Sensors_Sources.data[idx]);
  return e;
}

// Set default sensors for FanTemperatureControls.
// That means:
//   - Use "Average" as TemperatureAlgorithmType
//   - Utilize every sensor that is matched by `IsCPUSensorName`
static Error* FanTemperatureControl_SetDefaults(array_of(FanTemperatureControl)* fans) {
  Error* e;

  for_each_array(FanTemperatureControl*, ftc, *fans) {
    ftc->TemperatureAlgorithmType = TemperatureAlgorithmType_Average;
    ftc->TemperatureSourcesSize = 0;

    for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
      if (IsCPUSensorName(ts->name)) {
        e = FanTemperatureControl_AddTemperatureSource(ftc, ts);
        if (e)
          return e;
      }
    }
  }

  return err_success();
}

static Error* FanTemperatureControl_SetByModelConfig0(
  FanTemperatureControl* ftc,
  FanConfiguration* fc)
{
  Error* e;

  if (FanConfiguration_IsSet_TemperatureAlgorithmType(fc))
    ftc->TemperatureAlgorithmType = fc->TemperatureAlgorithmType;

  // Use default sensor names
  if (! fc->Sensors.size)
    return err_success();

  // Override sensors
  ftc->TemperatureSourcesSize = 0;

  for_each_array(const char**, sensor, fc->Sensors) {
    e = FanTemperatureControl_AddTemperatureSources(ftc, *sensor);
    if (e)
      return e;
  }

  return err_success();
}

// Set fan temperature sources by model config
static Error* FanTemperatureControl_SetByModelConfig(
  array_of(FanTemperatureControl)* fans,
  ModelConfig* model_config)
{
  Error* e;

  for_enumerate_array(int, fan_index, *fans) {
    FanTemperatureControl* ftc = &fans->data[fan_index];
    FanConfiguration* fc = &model_config->FanConfigurations.data[fan_index];

    e = FanTemperatureControl_SetByModelConfig0(ftc, fc);
    if (e)
      return err_stringf(e, "FanConfigurations[%d] (%s)", fan_index, fc->FanDisplayName);
  }

  return err_success();
}

// Initialize `fans` by `service_config`
static Error* FanTemperatureControl_SetByServiceConfig(
  array_of(FanTemperatureControl)* fans,
  ServiceConfig* service_config) 
{
  Error* e;

  for_each_array(FanTemperatureSourceConfig*, ftsc, service_config->FanTemperatureSources) {
    if (ftsc->FanIndex >= fans->size)
      return err_stringf(0, "Invalid FanIndex in FanTemperatureSources: %d", ftsc->FanIndex);

    FanTemperatureControl* ftc = &fans->data[ftsc->FanIndex];

    if (FanTemperatureSourceConfig_IsSet_TemperatureAlgorithmType(ftsc))
      ftc->TemperatureAlgorithmType = ftsc->TemperatureAlgorithmType;

    // If no sensors are given, use the defaults
    if (! ftsc->Sensors.size)
      continue;

    // Override sensors
    ftc->TemperatureSourcesSize = 0;

    for_each_array(const char**, sensor, ftsc->Sensors) {
      e = FanTemperatureControl_AddTemperatureSources(ftc, *sensor);
      if (e)
        return err_stringf(e, "FanTemperatureSources[%d]", ftsc->FanIndex);
    }
  }

  return err_success();
}

// Initialize temperature filters in `fans`
static Error* FanTemperatureControl_InitializeTemperatureFilters(
  array_of(FanTemperatureControl)* fans,
  int poll_interval)
{
  Error* e;

  for_each_array(FanTemperatureControl*, ftc, *fans) {
    e = TemperatureFilter_Init(&ftc->TemperatureFilter, poll_interval, NBFC_TEMPERATURE_FILTER_TIMESPAN);
    if (e)
      return e;
  }

  return err_success();
}

// Initialize FanTemperatureControls in `fans`
Error* FanTemperatureControl_Init(
  array_of(FanTemperatureControl)* fans,
  ServiceConfig* service_config,
  ModelConfig* model_config)
{
  Error* e;

  // Set default TemperatureAlgorithmType and temperature sources.
  e = FanTemperatureControl_SetDefaults(fans);
  if (e)
    return e;

  // Set temperature sources as specified in ModelConfig
  e = FanTemperatureControl_SetByModelConfig(fans, model_config);
  if (e)
    return e;

  // Set the temperature sources as specified in service_config 
  e = FanTemperatureControl_SetByServiceConfig(fans, service_config);
  if (e)
    return e;

  // Initialize the temperature filters
  e = FanTemperatureControl_InitializeTemperatureFilters(fans, model_config->EcPollInterval);
  if (e)
    return e;

  return err_success();
}

Error* FanTemperatureControl_UpdateFanTemperature(FanTemperatureControl* ftc) {
  float temp; // NOLINT
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
      Log_Info("Fan #%d (%s) uses '%s' (%s) as temperature source (%s)\n",
        fan_index,
        model_config->FanConfigurations.data[fan_index].FanDisplayName,
        ftc->TemperatureSources[i]->name,
        ftc->TemperatureSources[i]->file,
        TemperatureAlgorithmType_ToString(ftc->TemperatureAlgorithmType));
  }
}
