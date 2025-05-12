#ifndef NBFC_EC_DEBUG_H_
#define NBFC_EC_DEBUG_H_

#include "ec.h"

extern EC_VTable  EC_Debug_VTable;
extern EC_VTable* EC_Debug_Controller;

Error* EC_Debug_Open();
void   EC_Debug_Close();
Error* EC_Debug_WriteByte(uint8_t, uint8_t);
Error* EC_Debug_WriteWord(uint8_t, uint16_t);
Error* EC_Debug_ReadByte(uint8_t, uint8_t*);
Error* EC_Debug_ReadWord(uint8_t, uint16_t*);

#endif
