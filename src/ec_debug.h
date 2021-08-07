#ifndef NBFC_EC_DEBUG_H_
#define NBFC_EC_DEBUG_H_

#include "ec.h"

extern EC_VTable  EC_Debug_VTable;
extern EC_VTable* EC_Debug_Controller;

Error*   EC_Debug_Open();
void     EC_Debug_Close();
Error*   EC_Debug_WriteByte(int, uint8_t);
Error*   EC_Debug_WriteWord(int, uint16_t);
Error*   EC_Debug_ReadByte(int, uint8_t*);
Error*   EC_Debug_ReadWord(int, uint16_t*);

#endif
