#include "service.h"

#include "fan.h"
#ifdef HAVE_SENSORS
#include "lm_sensors.h"
#endif
#include "fs_sensors.h"
#include "temperature_filter.h"
#include "service_config.h"
#include "memory.h"
#include "macros.h"
#include "info.h"
#include "model_config.h"

#include <string.h> // memcpy
#include <unistd.h> // usleep
#include <math.h>   // fabs
#include <linux/limits.h>

Service_Options options;

static Config            model_config;
static Sensor_VTable*    sensor;
static TemperatureFilter temp_filter;
static array_of(Fan)     fans;
static int               Service_Initialized;

static Error* ApplyRegisterWriteConfig(int, uint8_t, RegisterWriteMode);
static Error* ApplyRegisterWriteConfgurations(bool);
static Error* ResetRegisterWriteConfigs();
static Error* ResetEC();
static Error* SetupEC(EmbeddedControllerType);

Error* Service_Init() {
  Error* e;

  e = ServiceConfig_Init(options.service_config);
  e_check();
  fprintf(stderr, "Using '%s' as model config\n", service_config.SelectedConfigId);

  char* path = (char*)Temp_Malloc(PATH_MAX, 1);
  snprintf(path, PATH_MAX, "./%s/%s.json", NBFC_CONFIGS_DIR, service_config.SelectedConfigId);
  e = Config_FromFile(&model_config, path);
  if (e) {
    snprintf(path, PATH_MAX, "%s/%s.json", NBFC_CONFIGS_DIR, service_config.SelectedConfigId);
    e = Config_FromFile(&model_config, path);
  }
  e_check();
  e = Config_Validate(&model_config);
  e_check();

  fans.size = model_config.FanConfigurations.size;
  fans.data = (Fan*) Mem_Calloc(fans.size, sizeof(Fan));

  for_enumerate_array(size_t, i, fans) {
    e = Fan_Init(
        &fans.data[i],
        &model_config.FanConfigurations.data[i],
        model_config.CriticalTemperature,
        model_config.ReadWriteWords
    );
    e_check();
  }

  for_enumerate_array(size_t, i, service_config.TargetFanSpeeds) {
    if (service_config.TargetFanSpeeds.data[i] >= 0.0f) {
      e = Fan_SetFixedSpeed(&fans.data[i], service_config.TargetFanSpeeds.data[i]);
      e_warn();
    }
    else
      Fan_SetAutoSpeed(&fans.data[i]);
  }

  if (options.embedded_controller_type != EmbeddedControllerType_Unset)
    e = SetupEC(options.embedded_controller_type); // --embedded-controller given
  else
    e = SetupEC(service_config.EmbeddedControllerType);
  e_check();

  e = (sensor = &FS_Sensors_VTable)->Init();
#ifdef HAVE_SENSORS
  if (e)
    e = (sensor = &LM_Sensors_VTable)->Init();
#endif
  e_check();

  e = Info_Init(options.state_file);
  e_check();
  e = TemperatureFilter_Init(&temp_filter, model_config.EcPollInterval, NBFC_TEMPERATURE_FILTER_TIMESPAN);
  e_check();

  if (options.read_only)
    e = ResetEC();
  else
    e = ApplyRegisterWriteConfgurations(true);
  e_check();

  if (options.fork)
    switch (fork()) {
    case -1: return err_stdlib(0, "fork");
    case 0:  break;
    default: _exit(0);
    }

  Service_Initialized = 1;
  return err_success();
}

Error* Service_Loop() {
  Error* e;
  float current_temperature;

  if ((e = sensor->GetTemperature(&current_temperature))) {
    e_warn();
    current_temperature = 1000;
  }
  current_temperature = TemperatureFilter_FilterTemperature(&temp_filter, current_temperature);

  bool re_init_required = false;
  for_each_array(Fan*, f, fans) {
    if ((e = Fan_UpdateCurrentSpeed(f)))
      e_warn();

    // Re-init if current fan speeds are off by more than 15%
    if (fabs(Fan_GetCurrentSpeed(f) - Fan_GetTargetSpeed(f)) > 15)
      re_init_required = true;
  }

  if (! options.read_only) {
    if (options.debug)
      fprintf(stderr, "re_init_required = 1;\n");

    e = ApplyRegisterWriteConfgurations(re_init_required);
    e_warn();
  }

  for_each_array(Fan*, f, fans) {
    Fan_SetTemperature(f, current_temperature);
    if (! options.read_only) {
      e = Fan_ECFlush(f);
      e_warn();
    }
  }

  e = Info_Write(&model_config, current_temperature, options.read_only, &fans);
  return e;
}

void Service_Error(Error* e) {
  static int failures;

  if (! e) {
    usleep(model_config.EcPollInterval * 1000);
    failures = 0;
    return;
  }

  if (++failures >= 100) {
    e_warn();
    fprintf(stderr, "We tried %d times, exiting now...\n", failures);
    exit(1);
  }

  usleep(10000);
}

static Error* SetupEC(EmbeddedControllerType ec_type) {
  switch (ec_type) {
    case EmbeddedControllerType_ECSysLinux:
      ec = &EC_SysLinux_VTable;
      fprintf(stderr, "Using 'ECSysLinux' as EmbeddedControllerType\n");
      break;
    case EmbeddedControllerType_ECLinux:
      ec = &EC_Linux_VTable;
      fprintf(stderr, "Using 'ECLinux' as EmbeddedControllerType\n");
      break;
    case EmbeddedControllerType_ECDummy:
      ec = &EC_Dummy_VTable;
      fprintf(stderr, "Using 'Dummy' as EmbeddedControllerType\n");
      break;
    case EmbeddedControllerType_Unset:
      abort();
  }

  if (options.debug) {
    EC_Debug_Controller = ec;
    ec = &EC_Debug_VTable;
  }

  return ec->Open();
}

static Error* ResetEC() {
  Error* e, *r = NULL;

  e_try_n(3, ResetRegisterWriteConfigs());
  if (e) r = e;

  for_each_array(Fan*, f, fans) {
    e_try_n(3, Fan_ECReset(f));
    if (e) r = e;
  }

  return r;
}

static Error* ResetRegisterWriteConfigs() {
  Error* e = NULL;
  for_each_array(RegisterWriteConfiguration*, cfg, model_config.RegisterWriteConfigurations)
    if (cfg->ResetRequired) {
      e = ApplyRegisterWriteConfig(cfg->Register, cfg->ResetValue, cfg->ResetWriteMode);
      e_warn();
    }
  return e;
}

static Error* ApplyRegisterWriteConfig(int register_, uint8_t value, RegisterWriteMode mode) {
  if (mode != RegisterWriteMode_Set) {
    uint8_t mask;
    Error* e = ec->ReadByte(register_, &mask);
    e_check();
    if (mode == RegisterWriteMode_And)
      value &= mask;
    else if (mode == RegisterWriteMode_Or)
      value |= mask;
  }

  return ec->WriteByte(register_, value);
}

static Error* ApplyRegisterWriteConfgurations(bool initializing) {
  for_each_array(RegisterWriteConfiguration*, cfg, model_config.RegisterWriteConfigurations) {
    if (initializing || cfg->WriteOccasion == RegisterWriteOccasion_OnWriteFanSpeed) {
       Error* e = ApplyRegisterWriteConfig(cfg->Register, cfg->Value, cfg->WriteMode);
       e_check();
    }
  }
  return err_success();
}

void Service_Cleanup() {
  Info_Close();
  if (Service_Initialized)
    if (ec) {
      ResetEC();
      ec->Close();
    }
  if (sensor)
    sensor->Cleanup();
}

