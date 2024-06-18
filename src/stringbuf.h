#ifndef NBFC_STRINGBUF_H_
#define NBFC_STRINGBUF_H_

#include <stdio.h>
#include <string.h>

typedef struct StringBuf StringBuf;
struct StringBuf {
  char* s;
  int   size; // strlen, (not including '\0')
  int   capacity; // including '\0'
};

#define StringBuf_Printf(S, ...) \
  StringBuf_Increment((S), snprintf((S)->s + (S)->size, (S)->capacity - (S)->size, __VA_ARGS__))

static inline void StringBuf_Increment(StringBuf* s, int len) {
  if (len < 0) {
    return; // -1 from printf
  }
  else if (s->size + 1 + len > s->capacity) {
    s->size = s->capacity - 1;
  }
  else {
    s->size += len;
  }

  s->s[s->size] = 0;
}

static inline void StringBuf_AddStr(StringBuf* s, const char* str) {
  StringBuf_Printf(s, "%s", str);
}

static inline void StringBuf_AddCh(StringBuf* s, int c) {
  if (s->size + 2 < s->capacity) {
    s->s[s->size++] = c;
    s->s[s->size] = 0;
  }
}

static inline int StringBuf_LastCh(StringBuf* s) {
  return s->size ? s->s[s->size -1] : -1;
}

static inline int StringBuf_PopCh(StringBuf* s) {
  if (s->size) {
    s->size--;
    int ret = s->s[s->size];
    s->s[s->size] = 0;
    return ret;
  }

  return -1;
}

#endif
