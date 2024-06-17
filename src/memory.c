#include "memory.h"

#include "macros.h" // unlikely
#include "nbfc.h"   // NBFC_EXIT_FATAL

#include <errno.h>  // errno
#include <stdlib.h> // malloc, calloc, realloc, free, exit
#include <string.h> // strerror
#include <stdio.h>  // fprintf

static void Mem_FatalError(const char* cause) {
  fprintf(stderr, "%s: %s\n", cause, strerror(errno));
  exit(NBFC_EXIT_FATAL);
}

void* Mem_Malloc(const size_t size) {
  void* p = malloc(size);
  if (unlikely(!p))
    Mem_FatalError("Mem_Malloc");
  return p;
}

void* Mem_Calloc(const size_t nmemb, const size_t size) {
  void* p = calloc(nmemb, size);
  if (unlikely(!p))
    Mem_FatalError("Mem_Calloc");
  return p;
}

void* Mem_Realloc(void* p, const size_t size) {
  void* new_p = realloc(p, size);
  if (unlikely(! new_p))
    Mem_FatalError("Mem_Realloc");
  return new_p;
}

char* Mem_Strdup(const char* s) {
  char* p = strdup(s);
  if (unlikely(!p))
    Mem_FatalError("Mem_Strdup");
  return p;
}

void Mem_Free(void* p) {
  free(p);
}
