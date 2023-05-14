#include "ec_dummy.h"

#include "memory.h"

static uint16_t* EC_Dummy_FakeRegisters = NULL;
#define          EC_Dummy_FakeRegistersSize 256

Error* EC_Dummy_Open() {
  if (! EC_Dummy_FakeRegisters)
    EC_Dummy_FakeRegisters = (uint16_t*) Mem_Calloc(EC_Dummy_FakeRegistersSize, sizeof(uint16_t));
  return err_success();
}

void EC_Dummy_Close() {
  Mem_Free(EC_Dummy_FakeRegisters);
  EC_Dummy_FakeRegisters = NULL;
}

Error* EC_Dummy_WriteByte(int register_, uint8_t value) {
  if (register_ < EC_Dummy_FakeRegistersSize)
    EC_Dummy_FakeRegisters[register_] = value;
  return err_success();
}

Error* EC_Dummy_WriteWord(int register_, uint16_t value) {
  if (register_ < EC_Dummy_FakeRegistersSize)
    EC_Dummy_FakeRegisters[register_] = value;
  return err_success();
}

Error* EC_Dummy_ReadByte(int register_, uint8_t* out) {
  *out = 0;
  if (register_ < EC_Dummy_FakeRegistersSize)
    *out = EC_Dummy_FakeRegisters[register_];
  return err_success();
}

Error* EC_Dummy_ReadWord(int register_, uint16_t* out) {
  *out = 0;
  if (register_ < EC_Dummy_FakeRegistersSize)
    *out = EC_Dummy_FakeRegisters[register_];
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

