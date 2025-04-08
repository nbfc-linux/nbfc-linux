#include "ec.h"

#include "ec_linux.h"
#include "ec_sys_linux.h"

bool EC_CheckWorking(EC_VTable* ec) {
  Error* e = ec->Open();
  if (e)
    return false;

  uint8_t byte;
  e = ec->ReadByte(0, &byte);
  ec->Close();
  return !e;
}

Error* EC_FindWorking(EC_VTable** out) {
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

  return err_string(0, "No working implementation found for accessing the embedded controller");
}
