#include "service.h"

#include "ec.h"
#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "ec_debug.h"
#include "ec_dummy.h"
#include "acpi_call.h"
#include "fan.h"
#include "fs_sensors.h"
#include "service_config.h"
#include "service_state.h"
#include "sponsor.h"
#include "nbfc.h"
#include "trace.h"
#include "memory.h"
#include "macros.h"
#include "model_config.h"

#include <stdio.h>  // snprintf
#include <math.h>   // fabs
#include <linux/limits.h> // PATH_MAX

Service_Options options;

extern EC_VTable* ec;

enum Service_Initialization {
  Initialized_0_None,
  Initialized_1_Service_Config,
  Initialized_2_Model_Config,
  Initialized_3_Sensors,
  Initialized_4_Fans,
  Initialized_5_Embedded_Controller,
  Initialized_6_Temperature_Filter,
};

ModelConfig              Service_Model_Config;
array_of(FanTemperatureControl) Service_Fans;
static enum Service_Initialization Service_State;

static Error* ApplyRegisterWriteConfigurations(bool);
static Error* ApplyRegisterWriteConfig(RegisterWriteConfiguration*);
static Error* ResetRegisterWriteConfigurations();
static Error* ResetRegisterWriteConfig(RegisterWriteConfiguration*);
static void   ResetEC();
static bool   IsAcpiCallUsed();
static EmbeddedControllerType EmbeddedControllerType_By_EC(EC_VTable*);
static EC_VTable* EC_By_EmbeddedControllerType(EmbeddedControllerType);

Error* Service_Init() {
  Error* e;
  Trace trace = {0};
  char path[PATH_MAX];
  Service_State = Initialized_0_None;

  // Service config ===========================================================
  e = ServiceConfig_Init(options.service_config);
  if (e) {
    goto error;
  }

  // Service state ============================================================
  ServiceState_Init(); // we don't care if this fails

  // Be backwards compatible
  if (ServiceConfig_IsSet_TargetFanSpeeds(&service_config)) {
    ServiceState_Set_TargetFanSpeeds(&service_state);
    service_state.TargetFanSpeeds = service_config.TargetFanSpeeds;

    ServiceConfig_UnSet_TargetFanSpeeds(&service_config);
    service_config.TargetFanSpeeds.data = NULL;
    service_config.TargetFanSpeeds.size = 0;
    ServiceConfig_Write(options.service_config);
  }

  Service_State = Initialized_1_Service_Config;

  // Model config =============================================================
  Log_Info("Using '%s' as model config\n", service_config.SelectedConfigId);
  e = ModelConfig_FindAndLoad(&Service_Model_Config, path, service_config.SelectedConfigId);
  if (e) {
    e = err_string(e, path);
    goto error;
  }

  Service_State = Initialized_2_Model_Config;

  Trace_Push(&trace, path);
  e = ModelConfig_Validate(&trace, &Service_Model_Config);
  if (e)
    goto error;

  Sponsor_Print();

  TemperatureThresholdManager_LegacyBehaviour = Service_Model_Config.LegacyTemperatureThresholdsBehaviour;

  // Sensor ===================================================================
  e = FS_Sensors_Init();
  if (e)
    goto error;
  FS_Sensors_Log();
  Service_State = Initialized_3_Sensors;

  // Fans =====================================================================
  Service_Fans.size = Service_Model_Config.FanConfigurations.size;
  Service_Fans.data = (FanTemperatureControl*) Mem_Calloc(Service_Fans.size, sizeof(FanTemperatureControl));
  Service_State = Initialized_4_Fans;

  for_enumerate_array(ssize_t, i, Service_Fans) {
    e = Fan_Init(
        &Service_Fans.data[i].Fan,
        &Service_Model_Config.FanConfigurations.data[i],
        &Service_Model_Config
    );
    if (e)
      goto error;
  }

  for_enumerate_array(ssize_t, i, service_state.TargetFanSpeeds) {
    if (i >= Service_Fans.size)
      continue;

    if (service_state.TargetFanSpeeds.data[i] >= 0.0f) {
      e = Fan_SetFixedSpeed(&Service_Fans.data[i].Fan, service_state.TargetFanSpeeds.data[i]);
      e_warn();
    }
    else
      Fan_SetAutoSpeed(&Service_Fans.data[i].Fan);
  }

  // Embedded controller ======================================================
  if (options.embedded_controller_type != EmbeddedControllerType_Unset) {
    // --embedded-controller given
    ec = EC_By_EmbeddedControllerType(options.embedded_controller_type);;
  }
  else if (ServiceConfig_IsSet_EmbeddedControllerType(&service_config)) {
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
#if ENABLE_EC_DEBUG
    EC_Debug_Controller = ec;
    ec = &EC_Debug_VTable;
#else
    Log_Warn("Debugging EC has been disabled at compile time.\n");
#endif
  }

  Service_State = Initialized_5_Embedded_Controller;

  // ACPI Call ================================================================
  if (IsAcpiCallUsed()) {
    e = AcpiCall_Open();
    if (e) {
      e = err_string(0, "Could not load kernel module 'acpi_call'. Is it installed?");
      goto error;
    }
  }

  // Register Write configurations ============================================
  if (! options.read_only) {
    e = ApplyRegisterWriteConfigurations(true);
    if (e)
      goto error;
  }

  // Initialize fans with sensors and temperature filter ======================
  e = FanTemperatureControl_Init(&Service_Fans, &service_config, &Service_Model_Config);
  if (e)
    goto error;
  Service_State = Initialized_6_Temperature_Filter;

  FanTemperatureControl_Log(&Service_Fans, &Service_Model_Config);

  return err_success();

error:
  Service_Cleanup();
  return e;
}

Error* Service_Loop() {
  Error* e = err_success();

  bool re_init_required = false;
  for_each_array(FanTemperatureControl*, f, Service_Fans) {
    e = Fan_UpdateCurrentSpeed(&f->Fan);
    if (e)
      goto error;

    // Re-init if current fan speeds are off by more than 15%
    if (fabs(Fan_GetCurrentSpeed(&f->Fan) - Fan_GetTargetSpeed(&f->Fan)) > 15) {
      re_init_required = true;
      Log_Debug("re_init_required = 1;\n");
    }
  }

  if (! options.read_only) {
    e = ApplyRegisterWriteConfigurations(re_init_required);
    if (e)
      goto error;
  }

  for_each_array(FanTemperatureControl*, ftc, Service_Fans) {
    e = FanTemperatureControl_UpdateFanTemperature(ftc);
    if (e)
      goto error;

    Fan_SetTemperature(&ftc->Fan, ftc->Temperature);
    if (! options.read_only) {
      e = Fan_ECFlush(&ftc->Fan);
      if (e)
        goto error;
    }
  }

error:
  return e;
}

static EmbeddedControllerType EmbeddedControllerType_By_EC(EC_VTable* ec) {
#if ENABLE_EC_SYS
  if (ec == &EC_SysLinux_VTable)       return EmbeddedControllerType_ECSysLinux;
#endif
#if ENABLE_EC_ACPI
  if (ec == &EC_SysLinux_ACPI_VTable)  return EmbeddedControllerType_ECSysLinuxACPI;
#endif
#if ENABLE_EC_DEV_PORT
  if (ec == &EC_Linux_VTable)          return EmbeddedControllerType_ECLinux;
#endif
#if ENABLE_EC_DUMMY
  if (ec == &EC_Dummy_VTable)          return EmbeddedControllerType_ECDummy;
#endif
  return EmbeddedControllerType_Unset;
}

static EC_VTable* EC_By_EmbeddedControllerType(EmbeddedControllerType t) {
  switch (t) {
#if ENABLE_EC_SYS
  case EmbeddedControllerType_ECSysLinux:     return &EC_SysLinux_VTable;
#endif
#if ENABLE_EC_ACPI
  case EmbeddedControllerType_ECSysLinuxACPI: return &EC_SysLinux_ACPI_VTable;
#endif
#if ENABLE_EC_DEV_PORT
  case EmbeddedControllerType_ECLinux:        return &EC_Linux_VTable;
#endif
#if ENABLE_EC_DUMMY
  case EmbeddedControllerType_ECDummy:        return &EC_Dummy_VTable;
#endif
  default: return NULL;
  }
}

static void ResetEC() {
  Error* e;
  bool failed = false;
  int tries = 10;

  do {
    e = ResetRegisterWriteConfigurations();
    e_warn();
    if (e)
      failed = true;

    for_each_array(FanTemperatureControl*, ftc, Service_Fans) {
      e = Fan_ECReset(&ftc->Fan);
      e_warn();
      if (e)
        failed = true;
    }
  } while (failed && --tries);
}

static Error* ResetRegisterWriteConfig(RegisterWriteConfiguration* cfg) {
  Error* e;
  uint8_t mask;
  uint64_t out;

  switch (cfg->ResetWriteMode) {
    case RegisterWriteMode_Set:
      return ec->WriteByte(cfg->Register, cfg->ResetValue);

    case RegisterWriteMode_And:
      e = ec->ReadByte(cfg->Register, &mask);
      e_check();
      return ec->WriteByte(cfg->Register, cfg->ResetValue & mask);

    case RegisterWriteMode_Or:
      e = ec->ReadByte(cfg->Register, &mask);
      e_check();
      return ec->WriteByte(cfg->Register, cfg->ResetValue | mask);

    case RegisterWriteMode_Call:
      e = AcpiCall_Call(cfg->ResetAcpiMethod, strlen(cfg->ResetAcpiMethod), &out);
      if (e)
        return err_string(e, "ResetAcpiMethod");
      else
        return err_success();

    default:
      return err_string(0, "ResetRegisterWriteConfig: INTERNAL ERROR");
  }
}

static Error* ResetRegisterWriteConfigurations() {
  Error* e = NULL;
  for_each_array(RegisterWriteConfiguration*, cfg, Service_Model_Config.RegisterWriteConfigurations)
    if (cfg->ResetRequired) {
      e = ResetRegisterWriteConfig(cfg);
      e_warn();
    }
  return e;
}

static Error* ApplyRegisterWriteConfig(RegisterWriteConfiguration* cfg) {
  Error* e;
  uint8_t mask;
  uint64_t out;

  switch (cfg->WriteMode) {
    case RegisterWriteMode_Set:
      return ec->WriteByte(cfg->Register, cfg->Value);

    case RegisterWriteMode_And:
      e = ec->ReadByte(cfg->Register, &mask);
      e_check();
      return ec->WriteByte(cfg->Register, cfg->Value & mask);

    case RegisterWriteMode_Or:
      e = ec->ReadByte(cfg->Register, &mask);
      e_check();
      return ec->WriteByte(cfg->Register, cfg->Value | mask);

    case RegisterWriteMode_Call:
      e = AcpiCall_Call(cfg->AcpiMethod, strlen(cfg->AcpiMethod), &out);
      if (e)
        return err_string(e, "AcpiMethod");
      else
        return err_success();

    default:
      return err_string(0, "ApplyRegisterWriteConfig: INTERNAL ERROR");
  }
}

static Error* ApplyRegisterWriteConfigurations(bool initializing) {
  for_each_array(RegisterWriteConfiguration*, cfg, Service_Model_Config.RegisterWriteConfigurations) {
    if (initializing || cfg->WriteOccasion == RegisterWriteOccasion_OnWriteFanSpeed) {
       Error* e = ApplyRegisterWriteConfig(cfg);
       e_check();
    }
  }
  return err_success();
}

static bool IsAcpiCallUsed() {
  for_each_array(FanConfiguration*, fc, Service_Model_Config.FanConfigurations) {
    if (FanConfiguration_IsSet_WriteAcpiMethod(fc))
      return true;

    if (FanConfiguration_IsSet_ReadAcpiMethod(fc))
      return true;

    if (FanConfiguration_IsSet_ResetAcpiMethod(fc))
      return true;
  }

  for_each_array(RegisterWriteConfiguration*, rwc, Service_Model_Config.RegisterWriteConfigurations) {
    if (rwc->WriteMode == RegisterWriteMode_Call)
      return true;

    if (rwc->ResetWriteMode == RegisterWriteMode_Call)
      return true;
  }

  return false;
}

void Service_WriteTargetFanSpeedsToState() {
  const int fancount = Service_Model_Config.FanConfigurations.size;

  service_state.TargetFanSpeeds.data = Mem_Realloc(service_state.TargetFanSpeeds.data, sizeof(float) * fancount);
  service_state.TargetFanSpeeds.size = fancount;

  for_enumerate_array(int, i, Service_Fans) {
    Fan* fan = &Service_Fans.data[i].Fan;
    if (fan->mode == Fan_ModeAuto)
      service_state.TargetFanSpeeds.data[i] = -1;
    else
      service_state.TargetFanSpeeds.data[i] = Fan_GetRequestedSpeed(fan);
  }
}

void Service_Cleanup() {
  switch (Service_State) {
    case Initialized_6_Temperature_Filter:
      for_each_array(FanTemperatureControl*, ftc, Service_Fans)
        TemperatureFilter_Close(&ftc->TemperatureFilter);
      /* fall through */
    case Initialized_5_Embedded_Controller:
      if (! options.read_only)
        ResetEC();
      ec->Close();
      /* fall through */
    case Initialized_4_Fans:
      Mem_Free(Service_Fans.data);
      /* fall through */
    case Initialized_3_Sensors:
      FS_Sensors_Cleanup();
      /* fall through */
    case Initialized_2_Model_Config:
      ModelConfig_Free(&Service_Model_Config);
      /* fall through */
    case Initialized_1_Service_Config:
      ServiceState_Write();
      ServiceState_Free();
      ServiceConfig_Free(&service_config);
      /* fall through */
    case Initialized_0_None:
      break;
  }

  Service_State = Initialized_0_None;
}
