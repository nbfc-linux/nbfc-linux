#ifndef NBFC_EC_H_
#define NBFC_EC_H_

#include "error.h"

#include <stdint.h>

typedef struct EC_VTable EC_VTable;
struct EC_VTable {
  Error*   (*Open)();
  void     (*Close)();
  Error*   (*ReadByte)(int, uint8_t*);
  Error*   (*ReadWord)(int, uint16_t*);
  Error*   (*WriteByte)(int, uint8_t);
  Error*   (*WriteWord)(int, uint16_t);
};

#endif
