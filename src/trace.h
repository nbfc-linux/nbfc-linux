#ifndef NBFC_TRACE_H_
#define NBFC_TRACE_H_

#include "macros.h"
#include <string.h> // memset

struct Trace {
  char buf[4096];
  size_t stack[32];
  size_t stack_size;
};
typedef struct Trace Trace;

void Trace_Push(Trace*, const char*, ...) PRINTF_LIKE(2, 3);
void Trace_Pop(Trace*);

static inline void Trace_Init(Trace* trace) {
  memset(trace, 0, sizeof(Trace));
}

#endif
