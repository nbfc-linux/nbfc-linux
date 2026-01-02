#ifndef NBFC_TRACE_H_
#define NBFC_TRACE_H_

#include <string.h> // memset

struct Trace {
  char buf[4096];
  unsigned int stack[32];
  unsigned int stack_size;
};
typedef struct Trace Trace;

void Trace_Push(Trace*, const char*, ...);
void Trace_Pop(Trace*);

static inline void Trace_Init(Trace* trace) {
  memset(trace, 0, sizeof(Trace));
}

#endif
