#ifndef NBFC_RANGE_H_
#define NBFC_RANGE_H_

#include "error.h"
#include "macros.h"

#include <stdint.h> // uint64_t
#include <stdbool.h>

typedef enum {
  Range_Add,
  Range_Sub,
  Range_Mul,
  Range_Div,
} Range_Type;

struct Range {
  uint64_t start;
  uint64_t stop;
  uint64_t step;
  Range_Type type;
};
typedef struct Range Range;
declare_array_of(Range);

Error    Range_Parse(const char*, Range*);
uint64_t Range_Begin(const Range*);
bool     Range_Iterate(const Range*, uint64_t*);
bool     Range_IsInRange(const Range*, uint64_t, uint64_t);

#endif
