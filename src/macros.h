#ifndef NBFC_MACROS_H_
#define NBFC_MACROS_H_

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h> // ssize_t

#define my               (*self)
#define max(A, B)        ((A) > (B) ? (A) : (B))
#define min(A, B)        ((A) < (B) ? (A) : (B))

#define PTR_DIFF(A, B) ((int) (A - B))

#define ARRAY_SIZE(A)    (sizeof(A) / sizeof(*A))
#define ARRAY_SSIZE(A)   ((ssize_t) ARRAY_SIZE(A))
#define array_of(T)      array_of_ ## T

#define range(TYPE, VAR, START, STOP) \
  TYPE VAR = START; VAR < STOP; ++VAR

#define for_enumerate_array(TYPE, VAR, ARRAY) \
  for (range(TYPE, VAR, 0, (ARRAY).size))

#define for_each_array(TYPE, VAR, ARRAY) \
  for (TYPE VAR = (ARRAY).data; VAR != (ARRAY).data + (ARRAY).size; ++VAR)

#define for_each_array_reverse(TYPE, VAR, ARRAY) \
  for (TYPE VAR = (ARRAY).data + (ARRAY).size; --VAR != (ARRAY).data;)

#define declare_array_of(T)                  \
  typedef struct array_of(T) array_of(T);    \
  struct array_of(T) {                       \
    T*  data;                                \
    ssize_t size;                            \
  }

declare_array_of(float);
declare_array_of(int);

#ifndef NDEBUG
#define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug(...) (void)0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#endif
