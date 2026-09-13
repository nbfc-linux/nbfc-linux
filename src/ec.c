#include "ec.h"

#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "ec_dummy.h"

bool EC_CheckWorking(const EC_VTable* ec) {
  Error e = ec->Open();
  if (e)
    return false;

  uint8_t byte;
  e = ec->ReadByte(0, &byte);
  ec->Close();
  return !e;
}

Error EC_FindWorking(const EC_VTable** out) {
#if ENABLE_EC_SYS
  if (EC_CheckWorking(&EC_SysLinux_VTable)) {
    *out = &EC_SysLinux_VTable;
    return err_success();
  }
#endif

#if ENABLE_EC_ACPI
  if (EC_CheckWorking(&EC_SysLinux_ACPI_VTable)) {
    *out = &EC_SysLinux_ACPI_VTable;
    return err_success();
  }
#endif

#if ENABLE_EC_DEV_PORT
  if (EC_CheckWorking(&EC_Linux_VTable)) {
    *out = &EC_Linux_VTable;
    return err_success();
  }
#endif

  return err_string("No working implementation found for accessing the embedded controller");
}

EmbeddedControllerType EmbeddedControllerType_By_EC(const EC_VTable* ec_vtable) {
#if ENABLE_EC_SYS
  if (ec_vtable == &EC_SysLinux_VTable)
    return EmbeddedControllerType_ECSysLinux;
#endif
#if ENABLE_EC_ACPI
  if (ec_vtable == &EC_SysLinux_ACPI_VTable)
    return EmbeddedControllerType_ECSysLinuxACPI;
#endif
#if ENABLE_EC_DEV_PORT
  if (ec_vtable == &EC_Linux_VTable)
    return EmbeddedControllerType_ECLinux;
#endif
#if ENABLE_EC_DUMMY
  if (ec_vtable == &EC_Dummy_VTable)
    return EmbeddedControllerType_ECDummy;
#endif
  return EmbeddedControllerType_Unset;
}

const EC_VTable* EC_By_EmbeddedControllerType(EmbeddedControllerType t) {
  switch (t) {
#if ENABLE_EC_SYS
  case EmbeddedControllerType_ECSysLinux:
    return &EC_SysLinux_VTable;
#endif
#if ENABLE_EC_ACPI
  case EmbeddedControllerType_ECSysLinuxACPI:
    return &EC_SysLinux_ACPI_VTable;
#endif
#if ENABLE_EC_DEV_PORT
  case EmbeddedControllerType_ECLinux:
    return &EC_Linux_VTable;
#endif
#if ENABLE_EC_DUMMY
  case EmbeddedControllerType_ECDummy:
    return &EC_Dummy_VTable;
#endif
  default:
    return NULL;
  }
}
