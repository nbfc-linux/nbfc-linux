#include "model_config_utils.h"

#include "acpi_analysis.h"

/*
 * Checks if two ACPI method calls are equal.
 *
 * TODO: Check if arguments are also equal
 */
static bool AcpiMethodCall_Equal(const char* a, const char* b) {
  return Acpi_Analysis_Path_Equals(a, b);
}

/*
 * Checks if two FanConfigurations are similar.
 */
static bool FanConfiguration_IsSimilar(
  const FanConfiguration* a,
  const FanConfiguration* b
) {
  const bool same_fields =
    (FanConfiguration_IsSet_ReadRegister(a) == FanConfiguration_IsSet_ReadRegister(b)) &&
    (FanConfiguration_IsSet_WriteRegister(a) == FanConfiguration_IsSet_WriteRegister(b)) &&
    (FanConfiguration_IsSet_ReadAcpiMethod(a) == FanConfiguration_IsSet_ReadAcpiMethod(b)) &&
    (FanConfiguration_IsSet_WriteAcpiMethod(a) == FanConfiguration_IsSet_WriteAcpiMethod(b)) &&
    (FanConfiguration_IsSet_ResetAcpiMethod(a) == FanConfiguration_IsSet_ResetAcpiMethod(b));

  if (! same_fields)
    return false;

  if (FanConfiguration_IsSet_ReadRegister(a))
    if (a->ReadRegister != b->ReadRegister)
      return false;

  if (FanConfiguration_IsSet_WriteRegister(a))
    if (a->WriteRegister != b->WriteRegister)
      return false;

  if (FanConfiguration_IsSet_ReadAcpiMethod(a))
    if (! AcpiMethodCall_Equal(a->ReadAcpiMethod, b->ReadAcpiMethod))
      return false;

  if (FanConfiguration_IsSet_WriteAcpiMethod(a))
    if (! AcpiMethodCall_Equal(a->WriteAcpiMethod, b->WriteAcpiMethod))
      return false;

  if (FanConfiguration_IsSet_ResetAcpiMethod(a))
    if (! AcpiMethodCall_Equal(a->ResetAcpiMethod, b->ResetAcpiMethod))
      return false;

  return true;
}

/*
 * Checks if two RegisterWriteConfigurations are similar.
 */
static bool RegisterWriteConfiguration_IsSimilar(
  const RegisterWriteConfiguration* a,
  const RegisterWriteConfiguration* b
) {
  const bool same_modes = 
    (a->WriteMode == b->WriteMode) &&
    (a->ResetWriteMode == b->ResetWriteMode);

  if (! same_modes)
    return false;

  if (a->WriteMode == RegisterWriteMode_Call) {
    if (! AcpiMethodCall_Equal(a->AcpiMethod, b->AcpiMethod))
      return false;
  }
  else {
    if (a->Register != b->Register)
      return false;
  }

  if (a->ResetWriteMode == RegisterWriteMode_Call) {
    if (! AcpiMethodCall_Equal(a->ResetAcpiMethod, b->ResetAcpiMethod))
      return false;
  }
  else {
    if (a->Register != b->Register)
      return false;
  }

  return true;
}

/*
 * Checks if two array_of(FanConfiguration) are similar.
 */
static bool FanConfigurations_AreSimilar(
  array_of(FanConfiguration)* fans1,
  array_of(FanConfiguration)* fans2
) {
  if (fans1->size != fans2->size)
    return false;

  bool used[4096] = {0};

  for_enumerate_array(array_size_t, fan_idx1, *fans1) {
    bool found = false;

    for_enumerate_array(array_size_t, fan_idx2, *fans2) {
      if (used[fan_idx2])
        continue;

      FanConfiguration* fan_cfg1 = &fans1->data[fan_idx1];
      FanConfiguration* fan_cfg2 = &fans2->data[fan_idx2];

      if (FanConfiguration_IsSimilar(fan_cfg1, fan_cfg2)) {
        used[fan_idx2] = true;
        found = true;
        break;
      }
    }

    if (! found)
      return false;
  }

  return true;
}

/*
 * Checks if two array_of(RegisterWriteConfiguration) are similar.
 */
static bool RegisterWriteConfigurations_AreSimilar(
  array_of(RegisterWriteConfiguration)* rwcs1,
  array_of(RegisterWriteConfiguration)* rwcs2
) {
  if (rwcs1->size != rwcs2->size)
    return false;

  bool used[4096] = {0};

  for_enumerate_array(array_size_t, rwc_idx1, *rwcs1) {
    bool found = false;

    for_enumerate_array(array_size_t, rwc_idx2, *rwcs2) {
      if (used[rwc_idx2])
        continue;

      RegisterWriteConfiguration* cfg1 = &rwcs1->data[rwc_idx1];
      RegisterWriteConfiguration* cfg2 = &rwcs2->data[rwc_idx2];

      if (RegisterWriteConfiguration_IsSimilar(cfg1, cfg2)) {
        used[rwc_idx2] = true;
        found = true;
        break;
      }
    }

    if (! found)
      return false;
  }

  return true;
}

/*
 * Checks if two ModelConfigs are similar.
 */
bool ModelConfig_IsSimilar(
  ModelConfig* a,
  ModelConfig* b 
) {
  return
    FanConfigurations_AreSimilar(&a->FanConfigurations, &b->FanConfigurations) &&
    RegisterWriteConfigurations_AreSimilar(&a->RegisterWriteConfigurations, &b->RegisterWriteConfigurations);
}
