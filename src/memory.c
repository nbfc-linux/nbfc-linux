#include "memory.h"

#include "macros.h" // unlikely
#include "nbfc.h"   // NBFC_EXIT_FATAL

#include <stdlib.h> // malloc, calloc, realloc, free, exit
#include <string.h> // strlen, strcpy, strerror
#include <stdio.h>  // fprintf
#include <errno.h>  // ENOMEM

static void Memory_FatalError() {
  fprintf(stderr, "FATAL ERROR: %s\n", strerror(ENOMEM));
  exit(NBFC_EXIT_FATAL);
}

#if MEMORY_TRACE_ALLOCS
void* Memory_Malloc(const char* trace, const size_t size)
#else
void* Memory_Malloc(const size_t size)
#endif
{

#if MEMORY_TRACE_ALLOCS
  fprintf(stderr, "MALLOC(%zu) [%s]\n", size, trace);
#endif

  void* p = calloc(1, size);
  if (unlikely(!p))
    Memory_FatalError();
  return p;
}

#if MEMORY_TRACE_ALLOCS
void* Memory_Calloc(const char* trace, const size_t nmemb, const size_t size)
#else
void* Memory_Calloc(const size_t nmemb, const size_t size)
#endif
{

#if MEMORY_TRACE_ALLOCS
  fprintf(stderr, "CALLOC(%zu, %zu) [%s]\n", nmemb, size, trace);
#endif

  void* p = calloc(nmemb, size);
  if (unlikely(!p))
    Memory_FatalError();
  return p;
}

#if MEMORY_TRACE_ALLOCS
void* Memory_Realloc(const char* trace, void* p, const size_t size)
#else
void* Memory_Realloc(void* p, const size_t size)
#endif
{

#if MEMORY_TRACE_ALLOCS
  fprintf(stderr, "REALLOC(%zu) [%s]\n", size, trace);
#endif

  void* new_p = realloc(p, size);
  if (unlikely(! new_p))
    Memory_FatalError();
  return new_p;
}

#if MEMORY_TRACE_ALLOCS
char* Memory_Strdup(const char* trace, const char* s)
#else
char* Memory_Strdup(const char* s)
#endif
{
  const size_t len = strlen(s);

#if MEMORY_TRACE_ALLOCS
  char* p = Memory_Malloc(trace, len + 1);
#else
  char* p = Memory_Malloc(len + 1);
#endif

  strcpy(p, s);
  return p;
}

void Memory_Free(void* p) {
  free(p);
}
