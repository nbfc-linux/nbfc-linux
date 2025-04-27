#include "trace.h"

#include "macros.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void Trace_Push(Trace* trace, const char* fmt, ...) {
  if (trace->stack_size >= ARRAY_SIZE(trace->stack))
    return;

  size_t len = strlen(trace->buf);
  trace->stack[trace->stack_size] = len;

  if (trace->stack_size) {
    trace->buf[len++] = ':';
    trace->buf[len++] = ' ';
  }

  va_list args;
  va_start(args, fmt);
  vsnprintf(trace->buf + len, sizeof(trace->buf) - len, fmt, args);
  va_end(args);

  trace->stack_size++;
}

void Trace_Pop(Trace* trace) {
  if (! trace->stack_size)
    return;

  const size_t previous_len = trace->stack[--trace->stack_size];
  trace->buf[previous_len] = '\0';
}
