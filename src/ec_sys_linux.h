#ifndef NBFC_EC_SYS_LINUX_H_
#define NBFC_EC_SYS_LINUX_H_

#include "ec.h"

extern EC_VTable EC_SysLinux_VTable;

Error*   EC_SysLinux_Init();
Error*   EC_SysLinux_Open();
void     EC_SysLinux_Close();
Error*   EC_SysLinux_WriteByte(int, uint8_t);
Error*   EC_SysLinux_WriteWord(int, uint16_t);
Error*   EC_SysLinux_ReadByte(int, uint8_t*);
Error*   EC_SysLinux_ReadWord(int, uint16_t*);

#endif
