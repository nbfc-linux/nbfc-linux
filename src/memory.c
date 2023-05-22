#include "memory.h"

#include "macros.h" // unlikely
#include "nbfc.h"

#include <errno.h>  // errno
#include <stdlib.h> // malloc
#include <string.h> // strerror
#include <stdio.h>

static inline void Memory_FatalError(const char* cause) {
  fprintf(stderr, "%s: %s\n", cause, strerror(errno));
  exit(NBFC_EXIT_FATAL);
}

void* Mem_Malloc(size_t size) {
  void* p = malloc(size);
  if (unlikely(!p))
    Memory_FatalError("Memory_Malloc");
  return p;
}

void* Mem_Calloc(const size_t nmemb, const size_t size) {
  void* p = calloc(nmemb, size);
  if (unlikely(!p))
    Memory_FatalError("Memory_Calloc");
  return p;
}

void* Mem_Realloc(void* p, const size_t size) {
  void* new_p = realloc(p, size);
  if (unlikely(! new_p))
    Memory_FatalError("Memory_Realloc");
  return new_p;
}

char* Mem_Strdup(const char* s) {
  char* p = strdup(s);
  if (unlikely(!p))
    Memory_FatalError("Memory_Strdup");
  return p;
}

void Mem_Free(void* p) {
  free(p);
}
