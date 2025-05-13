#include "ec_debug.h"

#include "log.h"

EC_VTable* EC_Debug_Controller;

Error* EC_Debug_Open() {
  return EC_Debug_Controller->Open();
}

void EC_Debug_Close() {
  EC_Debug_Controller->Close();
}

Error* EC_Debug_WriteByte(uint8_t register_, uint8_t value) {
  Error* e = EC_Debug_Controller->WriteByte(register_, value);
  Log_Debug("WriteByte(0x%X, 0x%X)\n", register_, value);
  e_warn();
  return e;
}

Error* EC_Debug_WriteWord(uint8_t register_, uint16_t value) {
  Error* e = EC_Debug_Controller->WriteWord(register_, value);
  Log_Debug("WriteWord(0x%X, 0x%X)\n", register_, value);
  e_warn();
  return e;
}

Error* EC_Debug_ReadByte(uint8_t register_, uint8_t* out) {
  Error* e = EC_Debug_Controller->ReadByte(register_, out);
  Log_Debug("ReadByte(0x%X, out = 0x%X)\n", register_, *out);
  e_warn();
  return e;
}

Error* EC_Debug_ReadWord(uint8_t register_, uint16_t* out) {
  Error* e = EC_Debug_Controller->ReadWord(register_, out);
  Log_Debug("ReadWord(0x%X, out = 0x%X)\n", register_, *out);
  e_warn();
  return e;
}

EC_VTable EC_Debug_VTable = {
  EC_Debug_Open,
  EC_Debug_Close,
  EC_Debug_ReadByte,
  EC_Debug_ReadWord,
  EC_Debug_WriteByte,
  EC_Debug_WriteWord,
};
