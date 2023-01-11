#ifndef EC_LINUX_H_
#define EC_LINUX_H_

#include "ec.h"

extern EC_VTable EC_Linux_VTable;

Error*   EC_Linux_Open();
void     EC_Linux_Close();
Error*   EC_Linux_WriteByte(int register_, uint8_t value);
Error*   EC_Linux_WriteWord(int register_, uint16_t value);
Error*   EC_Linux_ReadByte(int register_, uint8_t* out);
Error*   EC_Linux_ReadWord(int register_, uint16_t* out);

#endif
