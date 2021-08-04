#ifndef NBFC_STRINGBUF_H_
#define NBFC_STRINGBUF_H_

typedef struct StringBuf StringBuf;
struct StringBuf {
  char* s;
  int   size;
  int   capacity;
};

#define StringBuf_Printf(S, ...) \
  StringBuf_Increment((S), snprintf((S)->s + (S)->size, (S)->capacity, __VA_ARGS__))

static inline void StringBuf_AddCh(StringBuf* s, int c) {
  if (s->capacity) {
    s->s[s->size] = c;
    s->size++;
    s->capacity--;
    s->s[s->size] = 0;
  }
}

static inline void StringBuf_Increment(StringBuf* s, int len) {
  if (len < 0) {
    s->capacity = 0;
  }
  else if (len > s->capacity) {
    s->size += s->capacity;
    s->capacity = 0;
  }
  else {
    s->size += len;
    s->capacity -= len;
  }
  s->s[s->size] = 0;
}

#endif
