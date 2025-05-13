#ifndef NBFC_EC_H_
#define NBFC_EC_H_

#include "config.h"
#include "error.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct EC_VTable EC_VTable;
struct EC_VTable {
  Error* (*Open)();
  void   (*Close)();
  Error* (*ReadByte)(uint8_t, uint8_t*);
  Error* (*ReadWord)(uint8_t, uint16_t*);
  Error* (*WriteByte)(uint8_t, uint8_t);
  Error* (*WriteWord)(uint8_t, uint16_t);
};

bool   EC_CheckWorking(EC_VTable*);
Error* EC_FindWorking(EC_VTable**);

#endif
