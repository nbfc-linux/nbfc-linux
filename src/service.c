#include "service.h"

#include "ec.h"
#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "ec_debug.h"
#include "ec_dummy.h"
#include "fan.h"
#ifdef HAVE_SENSORS
#include "lm_sensors.h"
#endif
#include "fs_sensors.h"
#include "temperature_filter.h"
#include "service_config.h"
#include "nbfc.h"
#include "memory.h"
#include "macros.h"
#include "info.h"
#include "sleep.h"
#include "model_config.h"

#include <assert.h>
#include <stdio.h>
#include <math.h>   // fabs
#include <limits.h>
#include <unistd.h>

Service_Options options;

extern EC_VTable* ec;

enum Service_Initialization {
  Initialized_0_None,
  Initialized_1_Service_Config,
  Initialized_2_Model_Config,
  Initialized_3_Fans,
  Initialized_4_Embedded_Controller,
  Initialized_5_Sensors,
  Initialized_6_Temperature_Filter,
  Initialized_7_Info,
};

static ModelConfig       model_config;
static Sensor_VTable*    sensor;
static TemperatureFilter temp_filter;
static array_of(Fan)     fans;
static enum Service_Initialization Service_State;

static Error* ApplyRegisterWriteConfig(int, uint8_t, RegisterWriteMode);
static Error* ApplyRegisterWriteConfgurations(bool);
static Error* ResetRegisterWriteConfigs();
static Error* ResetEC();
static EmbeddedControllerType EmbeddedControllerType_By_EC(EC_VTable*);
static EC_VTable* EC_By_EmbeddedControllerType(EmbeddedControllerType);

Error* Service_Init() {
  Error* e;
  static char path[PATH_MAX];
  Service_State = Initialized_0_None;

  // Service config ===========================================================
  e = ServiceConfig_Init(options.service_config);
  if (e) {
    e = err_string(e, options.service_config);
    goto error;
  }
  Service_State = Initialized_1_Service_Config;

  // Model config =============================================================
  Log_Info("Using '%s' as model config\n", service_config.SelectedConfigId);

  snprintf(path, PATH_MAX, "%s/%s.json", NBFC_CONFIGS_DIR, service_config.SelectedConfigId);
  e = ModelConfig_FromFile(&model_config, path);
  if (e) {
    e = err_string(e, path);
    goto error;
  }
  Service_State = Initialized_2_Model_Config;

  e = ModelConfig_Validate(&model_config);
  if (e) {
    e = err_string(e, path);
    goto error;
  }

  // Fans =====================================================================
  fans.size = model_config.FanConfigurations.size;
  fans.data = (Fan*) Mem_Calloc(fans.size, sizeof(Fan));
  Service_State = Initialized_3_Fans;

  for_enumerate_array(size_t, i, fans) {
    e = Fan_Init(
        &fans.data[i],
        &model_config.FanConfigurations.data[i],
        model_config.CriticalTemperature,
        model_config.ReadWriteWords
    );
    if (e)
      goto error;
  }

  for_enumerate_array(size_t, i, service_config.TargetFanSpeeds) {
    if (service_config.TargetFanSpeeds.data[i] >= 0.0f) {
      e = Fan_SetFixedSpeed(&fans.data[i], service_config.TargetFanSpeeds.data[i]);
      e_warn();
    }
    else
      Fan_SetAutoSpeed(&fans.data[i]);
  }

  // Embedded controller ======================================================
  if (options.embedded_controller_type != EmbeddedControllerType_Unset) {
    // --embedded-controller given
    ec = EC_By_EmbeddedControllerType(options.embedded_controller_type);;
  }
  else if (service_config.EmbeddedControllerType != EmbeddedControllerType_Unset) {
    ec = EC_By_EmbeddedControllerType(service_config.EmbeddedControllerType);
  }
  else {
    e = EC_FindWorking(&ec);
    if (e)
      goto error;
  }

  EmbeddedControllerType t = EmbeddedControllerType_By_EC(ec);
  Log_Info("Using '%s' as EmbeddedControllerType\n", EmbeddedControllerType_ToString(t));
  e = ec->Open();
  if (e)
    goto error;

  if (options.debug) {
    EC_Debug_Controller = ec;
    ec = &EC_Debug_VTable;
  }

  Service_State = Initialized_4_Embedded_Controller;

  // Register Write configurations ============================================
  if (! options.read_only) {
    e = ApplyRegisterWriteConfgurations(true);
    if (e)
      goto error;
  }

  // Sensor ===================================================================
  e = (sensor = &FS_Sensors_VTable)->Init();
#ifdef HAVE_SENSORS
  if (e)
    e = (sensor = &LM_Sensors_VTable)->Init();
#endif
  if (e)
    goto error;
  Service_State = Initialized_5_Sensors;

  // Temperature Filter =======================================================
  e = TemperatureFilter_Init(&temp_filter, model_config.EcPollInterval, NBFC_TEMPERATURE_FILTER_TIMESPAN);
  if (e)
    goto error;
  Service_State = Initialized_6_Temperature_Filter;

  // Fork =====================================================================
  if (options.fork)
    switch (fork()) {
    case -1:
      e = err_stdlib(0, "fork");
      goto error;
    case 0:
      break;
    default:
      _exit(0);
    }

  // Info =====================================================================
  e = Info_Init(options.state_file);
  if (e)
    goto error;
  Service_State = Initialized_7_Info;

  return err_success();

error:
  Service_Cleanup();
  return e;
}

Error* Service_Loop() {
  Error* e;
  float current_temperature;
  static int failures = 0;

  e = sensor->GetTemperature(&current_temperature);
  if (e)
    goto error;

  current_temperature = TemperatureFilter_FilterTemperature(&temp_filter, current_temperature);

  bool re_init_required = false;
  for_each_array(Fan*, f, fans) {
    e = Fan_UpdateCurrentSpeed(f);
    if (e)
      goto error;

    // Re-init if current fan speeds are off by more than 15%
    if (fabs(Fan_GetCurrentSpeed(f) - Fan_GetTargetSpeed(f)) > 15) {
      re_init_required = true;

      if (options.debug)
        Log_Debug("re_init_required = 1;\n");
    }
  }

  if (! options.read_only) {
    e = ApplyRegisterWriteConfgurations(re_init_required);
    if (e)
      goto error;
  }

  for_each_array(Fan*, f, fans) {
    Fan_SetTemperature(f, current_temperature);
    if (! options.read_only) {
      e = Fan_ECFlush(f);
      if (e)
        goto error;
    }
  }

  e = Info_Write(&model_config, current_temperature, options.read_only, &fans);

error:
  if (! e) {
    sleep_ms(model_config.EcPollInterval);
    failures = 0;
  }
  else {
    if (++failures >= 100) {
      Log_Error("%s\n", err_print_all(e));
      Log_Error("We tried %d times, exiting now...\n", failures);
      exit(NBFC_EXIT_FAILURE);
    }

    sleep_ms(10);
  }
}

static EmbeddedControllerType EmbeddedControllerType_By_EC(EC_VTable* ec) {
  if (ec == &EC_SysLinux_VTable)       return EmbeddedControllerType_ECSysLinux;
  if (ec == &EC_SysLinux_ACPI_VTable)  return EmbeddedControllerType_ECSysLinuxACPI;
  if (ec == &EC_Linux_VTable)          return EmbeddedControllerType_ECLinux;
  if (ec == &EC_Dummy_VTable)          return EmbeddedControllerType_ECDummy;
  return EmbeddedControllerType_Unset;
}

static EC_VTable* EC_By_EmbeddedControllerType(EmbeddedControllerType t) {
  switch (t) {
  case EmbeddedControllerType_ECSysLinux: return &EC_SysLinux_VTable;
  case EmbeddedControllerType_ECSysLinuxACPI: return &EC_SysLinux_ACPI_VTable;
  case EmbeddedControllerType_ECLinux: return &EC_Linux_VTable;
  case EmbeddedControllerType_ECDummy: return &EC_Dummy_VTable;
  default: return NULL;
  }
}

static Error* ResetEC() {
  Error* e, *r = NULL;

  for (int tries = 3; tries; tries--) {
    e = ResetRegisterWriteConfigs();
    if (e) r = e;

    for_each_array(Fan*, f, fans) {
      e = Fan_ECReset(f);
      if (e) r = e;
    }
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
  switch (Service_State) {
    case Initialized_7_Info:
      Info_Close();
    case Initialized_6_Temperature_Filter:
      TemperatureFilter_Close(&temp_filter);
    case Initialized_5_Sensors:
      sensor->Cleanup();
    case Initialized_4_Embedded_Controller:
      if (! options.read_only)
        ResetEC();
      ec->Close();
    case Initialized_3_Fans:
      Mem_Free(fans.data);
    case Initialized_2_Model_Config:
      ModelConfig_Free(&model_config);
    case Initialized_1_Service_Config:
      ServiceConfig_Free(&service_config);
  }

  Service_State = Initialized_0_None;
}

