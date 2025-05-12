#ifndef EC_LINUX_H_
#define EC_LINUX_H_

#include "ec.h"

extern EC_VTable EC_Linux_VTable;

Error* EC_Linux_Open();
void   EC_Linux_Close();
Error* EC_Linux_WriteByte(uint8_t, uint8_t);
Error* EC_Linux_WriteWord(uint8_t, uint16_t);
Error* EC_Linux_ReadByte(uint8_t, uint8_t*);
Error* EC_Linux_ReadWord(uint8_t, uint16_t*);

#endif
