#include "../ec.h"
#include "../acpi_call.h"
#include "../fan.h"
#include "../nbfc.h"
#include "../macros.h"
#include "../model_config.h"
#include "../service_config.h"
#include "../register_write_configuration_utils.h"

#include "check_root.h"
#include "client_global.h"
#include "service_control.h"

static void ResetEc_ResetFans(ModelConfig* model_config) {
  Error e;

  for_each_array(FanConfiguration*, fan_config, model_config->FanConfigurations) {
    Fan fan = {0};

    e = Fan_Init(&fan, fan_config, model_config);
    if (e) {
      Log_Warn("Fan \"%s\": %s", fan_config->FanDisplayName, err_print_all(e));
      continue;
    }

    e = Fan_ECReset(&fan);
    if (e) {
      Log_Warn("Fan \"%s\": %s ", fan_config->FanDisplayName, err_print_all(e));
    }
  }
}

int ResetEc(void) {
  Error e;
  ServiceConfig service_config = {0};
  ModelConfig model_config = {0};

  check_root();

  // ==========================================================================
  // Load service configuration + model configuration
  // ==========================================================================
  Service_LoadAllConfigFiles(&service_config, &model_config);

  // ==========================================================================
  // Open embedded controller
  // ==========================================================================
  if (service_config.isset.EmbeddedControllerType) {
    ec = EC_By_EmbeddedControllerType(service_config.EmbeddedControllerType);
  }
  else {
    e = EC_FindWorking(&ec);
    if (e) {
      Log_Error("%s", err_print_all(e));
      return NBFC_EXIT_FAILURE;
    }
  }

  e = ec->Open();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Open ACPI Call
  // ==========================================================================
  AcpiCall_Open();

  // ==========================================================================
  // Do the reset
  // ==========================================================================
  for (int i = 0; i < 10; ++i) {
    RegisterWriteConfigurations_Reset(&model_config.RegisterWriteConfigurations);
    ResetEc_ResetFans(&model_config);
  }

  return NBFC_EXIT_SUCCESS;
}
