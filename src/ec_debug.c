#include "ec_debug.h"
#include "log.h"

#include <stdio.h>

EC_VTable* EC_Debug_Controller;

Error* EC_Debug_Open() {
  return EC_Debug_Controller->Open();
}

void EC_Debug_Close() {
  EC_Debug_Controller->Close();
}

Error* EC_Debug_WriteByte(int register_, uint8_t value) {
  Error* e = EC_Debug_Controller->WriteByte(register_, value);
  Log_Debug("WriteByte(%X, %X);\n", register_, value);
  e_warn();
  return e;
}

Error* EC_Debug_WriteWord(int register_, uint16_t value) {
  Error* e = EC_Debug_Controller->WriteWord(register_, value);
  Log_Debug("WriteWord(%X, %X);\n", register_, value);
  e_warn();
  return e;
}

Error* EC_Debug_ReadByte(int register_, uint8_t* out) {
  Error* e = EC_Debug_Controller->ReadByte(register_, out);
  Log_Debug("ReadByte(%X, out = %X);\n", register_, *out);
  e_warn();
  return e;
}

Error* EC_Debug_ReadWord(int register_, uint16_t* out) {
  Error* e = EC_Debug_Controller->ReadWord(register_, out);
  Log_Debug("ReadWord(%X, out = %X);\n", register_, *out);
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

