#ifndef NBFC_EC_CACHE_H_
#define NBFC_EC_CACHE_H_

#include "ec.h"

extern const EC_VTable  EC_Cache_VTable;
extern const EC_VTable* EC_Cache_Controller;

Error EC_Cache_Open(void);
void  EC_Cache_Close(void);
Error EC_Cache_WriteByte(uint8_t, uint8_t);
Error EC_Cache_WriteWord(uint8_t, uint16_t);
Error EC_Cache_ReadByte(uint8_t, uint8_t*);
Error EC_Cache_ReadWord(uint8_t, uint16_t*);
void  EC_Cache_Clear(void);

#endif
