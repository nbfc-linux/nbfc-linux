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
#include "buffer.h"
#include "macros.h"
#include "model_config.h"
#include "register_write_configuration_utils.h"

#include <stdio.h>  // snprintf
#include <math.h>   // fabs
#include <linux/limits.h> // PATH_MAX

Service_Options options;

extern const EC_VTable* ec;

enum Service_Initialization {
  Initialized_0_None,
  Initialized_1_Service_Config,
  Initialized_2_Model_Config,
  Initialized_3_Sensors,
  Initialized_4_Fans,
  Initialized_5_Embedded_Controller,
  Initialized_6_Temperature_Filter,
};

ModelConfig                        Service_ModelConfig = {0};
ServiceConfig                      Service_ServiceConfig = {0};
ServiceState                       Service_ServiceState = {0};
array_of(FanTemperatureControl)    Service_Fans = {0};
static enum Service_Initialization Service_State;

static void  ResetEC(void);
static bool  IsAcpiCallUsed(void);

Error Service_Init(void) {
  Error e;
  Trace* trace = (Trace*) Buffer_Get(sizeof(Trace));
  char* path = Buffer_Get(PATH_MAX);

  Trace_Init(trace);
  Service_State = Initialized_0_None;

  // Service config ===========================================================
  e = ServiceConfig_FromFile(&Service_ServiceConfig, options.service_config);
  if (e) {
    goto error;
  }

  // Service state ============================================================
  // (we don't care if this fails)
  ServiceState_FromFile(&Service_ServiceState, NBFC_STATE_FILE);

  // Be backwards compatible
  if (Service_ServiceConfig.isset.TargetFanSpeeds) {
    Service_ServiceState.isset.TargetFanSpeeds = true;
    Service_ServiceState.TargetFanSpeeds = Service_ServiceConfig.TargetFanSpeeds;

    Service_ServiceConfig.isset.TargetFanSpeeds = false;
    Service_ServiceConfig.TargetFanSpeeds.data = NULL;
    Service_ServiceConfig.TargetFanSpeeds.size = 0;
    ServiceConfig_Write(&Service_ServiceConfig, options.service_config);
  }

  Service_State = Initialized_1_Service_Config;

  // Model config =============================================================
  Log_Info("Using \"%s\" as model config", Service_ServiceConfig.SelectedConfigId);
  e = ModelConfig_FindAndLoad(&Service_ModelConfig, path, Service_ServiceConfig.SelectedConfigId);
  if (e) {
    e = err_chain_string(e, path);
    goto error;
  }

  Service_State = Initialized_2_Model_Config;

  Trace_Push(trace, "%s", path);
  e = ModelConfig_Validate(trace, &Service_ModelConfig);
  if (e)
    goto error;

  Sponsor_Print();

  TemperatureThresholdManager_LegacyBehaviour = Service_ModelConfig.LegacyTemperatureThresholdsBehaviour;

  // Sensor ===================================================================
  e = FS_Sensors_Init();
  if (e)
    goto error;
  FS_Sensors_Log();
  Service_State = Initialized_3_Sensors;

  // Fans =====================================================================
  Service_Fans.size = Service_ModelConfig.FanConfigurations.size;
  array_calloc(FanTemperatureControl, Service_Fans, Service_Fans.size);
  Service_State = Initialized_4_Fans;

  for_enumerate_array(array_size_t, i, Service_Fans) {
    e = Fan_Init(
        &Service_Fans.data[i].Fan,
        &Service_ModelConfig.FanConfigurations.data[i],
        &Service_ModelConfig
    );
    if (e)
      goto error;
  }

  for_enumerate_array(array_size_t, i, Service_ServiceState.TargetFanSpeeds) {
    if (i >= Service_Fans.size)
      continue;

    if (Service_ServiceState.TargetFanSpeeds.data[i] >= 0.0f) {
      e = Fan_SetFixedSpeed(&Service_Fans.data[i].Fan, Service_ServiceState.TargetFanSpeeds.data[i]);
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
  else if (Service_ServiceConfig.isset.EmbeddedControllerType) {
    ec = EC_By_EmbeddedControllerType(Service_ServiceConfig.EmbeddedControllerType);
  }
  else {
    e = EC_FindWorking(&ec);
    if (e)
      goto error;
  }

  EmbeddedControllerType t = EmbeddedControllerType_By_EC(ec);
  Log_Info("Using \"%s\" as EmbeddedControllerType", EmbeddedControllerType_ToString(t));
  e = ec->Open();
  if (e)
    goto error;

  if (options.debug) {
#if ENABLE_EC_DEBUG
    EC_Debug_Controller = ec;
    ec = &EC_Debug_VTable;
#else
    Log_Warn("Debugging EC has been disabled at compile time.");
#endif
  }

  Service_State = Initialized_5_Embedded_Controller;

  // ACPI Call ================================================================
  if (IsAcpiCallUsed()) {
    e = AcpiCall_Open();
    if (e) {
      e = err_string("Could not load kernel module \"acpi_call\". Is it installed?");
      goto error;
    }
  }

  // Register Write configurations ============================================
  if (! options.read_only) {
    e = RegisterWriteConfigurations_Apply(
          &Service_ModelConfig.RegisterWriteConfigurations, true);
    if (e)
      goto error;
  }

  // Initialize fans with sensors and temperature filter ======================
  e = FanTemperatureControl_Init(&Service_Fans, &Service_ServiceConfig, &Service_ModelConfig);
  if (e)
    goto error;
  Service_State = Initialized_6_Temperature_Filter;

  FanTemperatureControl_Log(&Service_Fans, &Service_ModelConfig);

error:

  Buffer_Release(path, PATH_MAX);
  Buffer_Release((char*) trace, sizeof(Trace));

  if (e)
    Service_Cleanup();

  return e;
}

Error Service_Loop(void) {
  Error e = err_success();

  bool re_init_required = false;
  for_each_array(FanTemperatureControl*, f, Service_Fans) {
    e = Fan_UpdateCurrentSpeed(&f->Fan);
    if (e)
      goto error;

    // Re-init if current fan speeds are off by more than 15%
    if (fabs(Fan_GetCurrentSpeed(&f->Fan) - Fan_GetTargetSpeed(&f->Fan)) > 15) {
      re_init_required = true;
      Log_Debug("re_init_required = 1;");
    }
  }

  if (! options.read_only) {
    e = RegisterWriteConfigurations_Apply(
          &Service_ModelConfig.RegisterWriteConfigurations, re_init_required);

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

static void ResetEC(void) {
  Error e;
  bool failed = false;
  int tries = 10;

  do {
    e = RegisterWriteConfigurations_Reset(&Service_ModelConfig.RegisterWriteConfigurations);
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

static bool IsAcpiCallUsed(void) {
  for_each_array(FanConfiguration*, fc, Service_ModelConfig.FanConfigurations) {
    if (fc->isset.WriteAcpiMethod)
      return true;

    if (fc->isset.ReadAcpiMethod)
      return true;

    if (fc->isset.ResetAcpiMethod)
      return true;
  }

  for_each_array(RegisterWriteConfiguration*, rwc, Service_ModelConfig.RegisterWriteConfigurations) {
    if (rwc->WriteMode == RegisterWriteMode_Call)
      return true;

    if (rwc->ResetWriteMode == RegisterWriteMode_Call)
      return true;
  }

  return false;
}

void Service_WriteTargetFanSpeedsToState(void) {
  const array_size_t fancount = Service_ModelConfig.FanConfigurations.size;

  array_realloc(float, Service_ServiceState.TargetFanSpeeds, fancount);
  Service_ServiceState.TargetFanSpeeds.size = fancount;

  for_enumerate_array(array_size_t, i, Service_Fans) {
    Fan* fan = &Service_Fans.data[i].Fan;
    if (fan->mode == Fan_ModeAuto)
      Service_ServiceState.TargetFanSpeeds.data[i] = -1;
    else
      Service_ServiceState.TargetFanSpeeds.data[i] = Fan_GetRequestedSpeed(fan);
  }
}

void Service_Cleanup(void) {
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
      ModelConfig_Free(&Service_ModelConfig);
      /* fall through */
    case Initialized_1_Service_Config:
      ServiceState_Write(&Service_ServiceState, NBFC_STATE_FILE);
      ServiceState_Free(&Service_ServiceState);
      ServiceConfig_Free(&Service_ServiceConfig);
      /* fall through */
    case Initialized_0_None:
      break;
  }

  Service_State = Initialized_0_None;
}
