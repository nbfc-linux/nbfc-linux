#ifndef NBFC_EC_SYS_LINUX_H_
#define NBFC_EC_SYS_LINUX_H_

#include "ec.h"

extern EC_VTable EC_SysLinux_VTable;
extern EC_VTable EC_SysLinux_ACPI_VTable;

Error* EC_SysLinux_Open();
Error* EC_SysLinux_ACPI_Open();
void   EC_SysLinux_Close();
Error* EC_SysLinux_WriteByte(uint8_t, uint8_t);
Error* EC_SysLinux_WriteWord(uint8_t, uint16_t);
Error* EC_SysLinux_ReadByte(uint8_t, uint8_t*);
Error* EC_SysLinux_ReadWord(uint8_t, uint16_t*);

#endif
