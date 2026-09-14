#include "fan_temperature_control.h"

#include "nbfc.h"
#include "memory.h"

#include <string.h>

// Set default sensors for FanTemperatureControls.
// That means:
//   - Use "Average" as TemperatureAlgorithmType
//   - Utilize every CPU sensor
static Error FanTemperatureControl_SetDefaults(array_of(FanTemperatureControl)* fans) {
  for_each_array(FanTemperatureControl*, ftc, *fans) {
    ftc->TemperatureAlgorithmType = TemperatureAlgorithmType_Average;
    ftc->TemperatureSources.size = 0;
    FS_TemperatureSources_AddTemperatureSources(&ftc->TemperatureSources, "@CPU");
  }

  return err_success();
}

static Error FanTemperatureControl_SetByModelConfig0(
  FanTemperatureControl* ftc,
  FanConfiguration* fc)
{
  Error e;

  if (fc->isset.TemperatureAlgorithmType)
    ftc->TemperatureAlgorithmType = fc->TemperatureAlgorithmType;

  // Use default sensor names
  if (! fc->Sensors.size)
    return err_success();

  // Override sensors
  ftc->TemperatureSources.size = 0;

  for_each_array(const char**, sensor, fc->Sensors) {
    e = FS_TemperatureSources_AddTemperatureSources(&ftc->TemperatureSources, *sensor);
    if (e)
      return e;
  }

  return err_success();
}

// Set fan temperature sources by model config
static Error FanTemperatureControl_SetByModelConfig(
  array_of(FanTemperatureControl)* fans,
  ModelConfig* model_config)
{
  Error e;

  for_enumerate_array(array_size_t, fan_index, *fans) {
    FanTemperatureControl* ftc = &fans->data[fan_index];
    FanConfiguration* fc = &model_config->FanConfigurations.data[fan_index];

    e = FanTemperatureControl_SetByModelConfig0(ftc, fc);
    if (e)
      return err_chain_stringf(e, "FanConfigurations[%zd] (%s)", fan_index, fc->FanDisplayName);
  }

  return err_success();
}

// Initialize `fans` by `service_config`
static Error FanTemperatureControl_SetByServiceConfig(
  array_of(FanTemperatureControl)* fans,
  ServiceConfig* service_config)
{
  Error e;

  for_each_array(FanTemperatureSourceConfig*, ftsc, service_config->FanTemperatureSources) {
    if (ftsc->FanIndex >= fans->size)
      return err_stringf("Invalid FanIndex in FanTemperatureSources: %d", ftsc->FanIndex);

    FanTemperatureControl* ftc = &fans->data[ftsc->FanIndex];

    if (ftsc->isset.TemperatureAlgorithmType)
      ftc->TemperatureAlgorithmType = ftsc->TemperatureAlgorithmType;

    // If no sensors are given, use the defaults
    if (! ftsc->Sensors.size)
      continue;

    // Override sensors
    ftc->TemperatureSources.size = 0;

    for_each_array(const char**, sensor, ftsc->Sensors) {
      e = FS_TemperatureSources_AddTemperatureSources(&ftc->TemperatureSources, *sensor);
      if (e)
        return err_chain_stringf(e, "FanTemperatureSources[%d]", ftsc->FanIndex);
    }
  }

  return err_success();
}

// Initialize temperature filters in `fans`
static Error FanTemperatureControl_InitializeTemperatureFilters(
  array_of(FanTemperatureControl)* fans,
  unsigned int poll_interval)
{
  Error e;

  for_each_array(FanTemperatureControl*, ftc, *fans) {
    e = TemperatureFilter_Init(&ftc->TemperatureFilter, poll_interval, NBFC_TEMPERATURE_FILTER_TIMESPAN);
    if (e)
      return e;
  }

  return err_success();
}

// Initialize FanTemperatureControls in `fans`
Error FanTemperatureControls_Init(
  array_of(FanTemperatureControl)* fans,
  ServiceConfig* service_config,
  ModelConfig* model_config)
{
  Error e;

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

Error FanTemperatureControl_UpdateFanTemperature(FanTemperatureControl* ftc) {
  Error e;
  float temp; // NOLINT

  e = FS_TemperatureSources_GetTemperature(
        &ftc->TemperatureSources, ftc->TemperatureAlgorithmType, &temp);

  if (e)
    return e;

  ftc->Temperature = TemperatureFilter_FilterTemperature(&ftc->TemperatureFilter, temp);
  return err_success();
}

void FanTemperatureControls_Log(const array_of(FanTemperatureControl)* fans, ModelConfig* model_config) {
  for_enumerate_array(array_size_t, fan_index, *fans) {
    FanTemperatureControl* ftc = &fans->data[fan_index];

    for_each_array(array_size_t*, ts_ref, ftc->TemperatureSources) {
      const FS_TemperatureSource* ts = FS_Sensors_Sources_UnRef(*ts_ref);

      Log_Info("Fan #%zd (%s) uses \"%s\" (%s) as temperature source (%s)",
        fan_index,
        model_config->FanConfigurations.data[fan_index].FanDisplayName,
        ts->name,
        ts->file,
        TemperatureAlgorithmType_ToString(ftc->TemperatureAlgorithmType));
    }
  }
}

void FanTemperatureControls_Free(array_of(FanTemperatureControl)* ftcs) {
  for_each_array(FanTemperatureControl*, ftc, *ftcs)
    Mem_Free(ftc->TemperatureSources.data);
  Mem_Free(ftcs->data);
}
