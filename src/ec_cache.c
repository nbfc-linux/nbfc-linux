#include "ec_cache.h"

#include <stdbool.h>

const EC_VTable* EC_Cache_Controller;

static uint8_t EC_Cache_Cache[256];
static bool    EC_Cache_IsSet[256] = {0};

void EC_Cache_Clear(void) {
  memset(EC_Cache_IsSet, 0, 256);
}

Error EC_Cache_Open(void) {
  return EC_Cache_Controller->Open();
}

void EC_Cache_Close(void) {
  EC_Cache_Controller->Close();
}

Error EC_Cache_WriteByte(uint8_t register_, uint8_t value) {
  return err_string("NOT SUPPORTED");
}

Error EC_Cache_WriteWord(uint8_t register_, uint16_t value) {
  return err_string("NOT SUPPORTED");
}

Error EC_Cache_ReadByte(uint8_t register_, uint8_t* out) {
  Error e;

  if (EC_Cache_IsSet[register_]) {
    *out = EC_Cache_Cache[register_];
    return err_success();
  }

  e = EC_Cache_Controller->ReadByte(register_, out);
  if (e)
    return e;

  EC_Cache_IsSet[register_] = true;
  EC_Cache_Cache[register_] = *out;
  return err_success();
}

Error EC_Cache_ReadWord(uint8_t register_, uint16_t* out) {
  Error e;
  uint8_t lsb;
  uint8_t msb;

  if (register_ > 254)
    return err_string("EC_Cache_ReadWord(): register > 254");

  e = EC_Cache_ReadByte(register_ + 0, &lsb);
  if (e)
    return e;

  e = EC_Cache_ReadByte(register_ + 1, &msb);
  if (e)
    return e;

  *out = (uint16_t) (((uint16_t) lsb) | (((uint16_t) msb) << 8));
  return err_success();
}

const EC_VTable EC_Cache_VTable = {
  EC_Cache_Open,
  EC_Cache_Close,
  EC_Cache_ReadByte,
  EC_Cache_ReadWord,
  EC_Cache_WriteByte,
  EC_Cache_WriteWord,
};
