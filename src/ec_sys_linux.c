#undef _XOPEN_SOURCE
#undef _DEFAULT_SOURCE

#define _XOPEN_SOURCE 500 // unistd.h: pwrite()/pread()
#define _DEFAULT_SOURCE   // endian.h: htole16(), le16toh()

#include "ec_sys_linux.h"

#include <endian.h> // htole16, le16toh
#include <fcntl.h>  // open, close, O_RDWR
#include <stdlib.h> // system
#include <unistd.h> // pread, pwrite

#define EC_SysLinux_ACPI_EC_Path    "/dev/ec"
#define EC_SysLinux_EC0_IO_Path     "/sys/kernel/debug/ec/ec0/io"
#define EC_SysLinux_ACPI_Module_Cmd "modprobe acpi_ec write_support=1"
#define EC_SysLinux_Module_Cmd      "modprobe ec_sys write_support=1"

static int         EC_SysLinux_FD = -1;
static const char* EC_SysLinux_File = NULL;

static inline Error* EC_SysLinux_LoadKernelModule();
static inline Error* EC_SysLinux_LoadACPIKernelModule();

Error* EC_SysLinux_Open() {
  EC_SysLinux_File = EC_SysLinux_EC0_IO_Path;

  EC_SysLinux_FD = open(EC_SysLinux_EC0_IO_Path, O_RDWR);
  if (EC_SysLinux_FD != -1)
    return err_success();

  Error* e = EC_SysLinux_LoadKernelModule();
  e_check();

  EC_SysLinux_FD = open(EC_SysLinux_EC0_IO_Path, O_RDWR);
  if (EC_SysLinux_FD == -1)
    return err_stdlib(0, EC_SysLinux_EC0_IO_Path);
  else
    return err_success();
}

Error* EC_SysLinux_ACPI_Open() {
  EC_SysLinux_File = EC_SysLinux_ACPI_EC_Path;

  EC_SysLinux_FD = open(EC_SysLinux_ACPI_EC_Path, O_RDWR);
  if (EC_SysLinux_FD != -1)
    return err_success();

  Error* e = EC_SysLinux_LoadACPIKernelModule();
  e_check();

  EC_SysLinux_FD = open(EC_SysLinux_ACPI_EC_Path, O_RDWR);
  if (EC_SysLinux_FD == -1)
    return err_stdlib(0, EC_SysLinux_ACPI_EC_Path);
  else
    return err_success();
}

void EC_SysLinux_Close() {
  if (EC_SysLinux_FD > -1) {
    close(EC_SysLinux_FD);
    EC_SysLinux_FD = -1;
  }
}

Error* EC_SysLinux_WriteByte(uint8_t register_, uint8_t value) {
  if (1 != pwrite(EC_SysLinux_FD, &value, 1, register_))
    return err_stdlib(0, EC_SysLinux_File);
  return err_success();
}

Error* EC_SysLinux_WriteWord(uint8_t register_, uint16_t value) {
  value = htole16(value);
  if (2 != pwrite(EC_SysLinux_FD, &value, 2, register_))
    return err_stdlib(0, EC_SysLinux_File);
  return err_success();
}

Error* EC_SysLinux_ReadByte(uint8_t register_, uint8_t* out) {
  uint8_t value;
  if (1 != pread(EC_SysLinux_FD, &value, 1, register_))
    return err_stdlib(0, EC_SysLinux_File);
  *out = value;
  return err_success();
}

Error* EC_SysLinux_ReadWord(uint8_t register_, uint16_t* out) {
  uint16_t value;
  if (2 != pread(EC_SysLinux_FD, &value, 2, register_))
    return err_stdlib(0, EC_SysLinux_File);
  *out = le16toh(value);
  return err_success();
}

static inline Error* EC_SysLinux_LoadKernelModule() {
  switch (system(EC_SysLinux_Module_Cmd)) {
  case 0:  return err_success();
  case -1: return err_stdlib(0, "system()");
  default: return err_stringf(0, "Could not execute `%s'", EC_SysLinux_Module_Cmd);
  }
}

static inline Error* EC_SysLinux_LoadACPIKernelModule() {
  switch (system(EC_SysLinux_ACPI_Module_Cmd)) {
  case 0:  return err_success();
  case -1: return err_stdlib(0, "system()");
  default: return err_stringf(0, "Could not execute `%s'", EC_SysLinux_ACPI_Module_Cmd);
  }
}

EC_VTable EC_SysLinux_VTable = {
  EC_SysLinux_Open,
  EC_SysLinux_Close,
  EC_SysLinux_ReadByte,
  EC_SysLinux_ReadWord,
  EC_SysLinux_WriteByte,
  EC_SysLinux_WriteWord,
};

EC_VTable EC_SysLinux_ACPI_VTable = {
  EC_SysLinux_ACPI_Open,
  EC_SysLinux_Close,
  EC_SysLinux_ReadByte,
  EC_SysLinux_ReadWord,
  EC_SysLinux_WriteByte,
  EC_SysLinux_WriteWord,
};
