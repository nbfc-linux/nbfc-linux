#ifndef NBFC_EC_DUMMY_H_
#define NBFC_EC_DUMMY_H_

#include "ec.h"

extern EC_VTable EC_Dummy_VTable;

Error* EC_Dummy_Open();
void   EC_Dummy_Close();
Error* EC_Dummy_WriteByte(uint8_t, uint8_t);
Error* EC_Dummy_WriteWord(uint8_t, uint16_t);
Error* EC_Dummy_ReadByte(uint8_t, uint8_t*);
Error* EC_Dummy_ReadWord(uint8_t, uint16_t*);

#endif
