#ifndef NBFC_STRINGBUF_H_
#define NBFC_STRINGBUF_H_

#include "macros.h"

#include <stdio.h>  // vsnprintf
#include <stdarg.h> // va_list, va_start, va_end

typedef struct StringBuf StringBuf;
struct StringBuf {
  char* s;
  int len;
  int capacity; // including '\0'
};

static inline void PRINTF_LIKE(2, 3) StringBuf_Printf(StringBuf* s, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  size_t remaining = (size_t) (s->capacity - s->len);
  int written = vsnprintf(s->s + s->len, remaining, fmt, ap);

  if (written < 0)
    ;
  else if (s->len + written >= s->capacity)
    s->len = s->capacity - 1;
  else
    s->len += written;

  s->s[s->len] = '\0';

  va_end(ap);
}

static inline void StringBuf_AddStr(StringBuf* s, const char* string) {
  StringBuf_Printf(s, "%s", string);
}

static inline void StringBuf_AddCh(StringBuf* s, char c) {
  if (s->len + 1 < s->capacity) {
    s->s[s->len++] = c;
    s->s[s->len] = 0;
  }
}

static inline int StringBuf_LastCh(StringBuf* s) {
  return s->len ? s->s[s->len - 1] : -1;
}

static inline int StringBuf_PopCh(StringBuf* s) {
  if (s->len) {
    s->len--;
    int ret = s->s[s->len];
    s->s[s->len] = '\0';
    return ret;
  }

  return -1;
}

#endif
