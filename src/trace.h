#ifndef TRACE_H_
#define TRACE_H_

struct Trace {
  char buf[4096];
  unsigned int stack[32];
  unsigned int stack_size;
};
typedef struct Trace Trace;

void Trace_Push(Trace*, const char*, ...);
void Trace_Pop(Trace*);

#endif
