#ifndef TRACE_H_
#define TRACE_H_

#include <stddef.h>

struct Trace {
  char buf[4096];
  size_t stack[32];
  size_t stack_size;
};
typedef struct Trace Trace;

void Trace_Push(Trace*, const char*, ...);
void Trace_Pop(Trace*);

#endif
