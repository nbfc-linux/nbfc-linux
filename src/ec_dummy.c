#undef _DEFAULT_SOURCE

#define _DEFAULT_SOURCE // endian.h: htole16

#include "ec_dummy.h"

#include <endian.h> // htole16

#include "memory.h"

static uint8_t* EC_Dummy_FakeRegisters = NULL;
#define         EC_Dummy_FakeRegistersSize 256

Error* EC_Dummy_Open() {
  if (! EC_Dummy_FakeRegisters)
    EC_Dummy_FakeRegisters = (uint8_t*) Mem_Calloc(EC_Dummy_FakeRegistersSize, sizeof(uint8_t));
  return err_success();
}

void EC_Dummy_Close() {
  Mem_Free(EC_Dummy_FakeRegisters);
  EC_Dummy_FakeRegisters = NULL;
}

Error* EC_Dummy_ReadByte(uint8_t register_, uint8_t* out) {
  *out = 0;
  if (register_ < EC_Dummy_FakeRegistersSize)
    *out = EC_Dummy_FakeRegisters[register_];
  return err_success();
}

Error* EC_Dummy_WriteByte(uint8_t register_, uint8_t value) {
  if (register_ < EC_Dummy_FakeRegistersSize)
    EC_Dummy_FakeRegisters[register_] = value;
  return err_success();
}

Error* EC_Dummy_ReadWord(uint8_t register_, uint16_t* out) {
  if (register_ + 1 < EC_Dummy_FakeRegistersSize) {
    *out = ((uint16_t) EC_Dummy_FakeRegisters[register_]) |
          (((uint16_t) EC_Dummy_FakeRegisters[register_+1]) << 8);
  }
  return err_success();
}

Error* EC_Dummy_WriteWord(uint8_t register_, uint16_t value) {
  value = htole16(value);

  uint8_t msb = value >> 8;
  uint8_t lsb = value;

  if (register_ + 1 < EC_Dummy_FakeRegistersSize) {
    EC_Dummy_FakeRegisters[register_] = lsb;
    EC_Dummy_FakeRegisters[register_ + 1] = msb;
  }
  return err_success();
}

EC_VTable EC_Dummy_VTable = {
  EC_Dummy_Open,
  EC_Dummy_Close,
  EC_Dummy_ReadByte,
  EC_Dummy_ReadWord,
  EC_Dummy_WriteByte,
  EC_Dummy_WriteWord,
};
