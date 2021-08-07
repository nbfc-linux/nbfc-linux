#ifndef NBFC_EC_DUMMY_H_
#define NBFC_EC_DUMMY_H_

#include "ec.h"

extern EC_VTable EC_Dummy_VTable;

Error*   EC_Dummy_Open();
void     EC_Dummy_Close();
Error*   EC_Dummy_WriteByte(int, uint8_t);
Error*   EC_Dummy_WriteWord(int, uint16_t);
Error*   EC_Dummy_ReadByte(int, uint8_t*);
Error*   EC_Dummy_ReadWord(int, uint16_t*);

#endif
