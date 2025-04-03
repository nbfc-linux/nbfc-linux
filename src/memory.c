#undef _XOPEN_SOURCE

#define _XOPEN_SOURCE 500 // string.h: strdup()

#include "memory.h"

#include "macros.h" // unlikely
#include "nbfc.h"   // NBFC_EXIT_FATAL

#include <stdlib.h> // malloc, calloc, realloc, free, exit
#include <string.h> // strerror
#include <stdio.h>  // fprintf
#include <errno.h>  // ENOMEM

static void Mem_FatalError() {
  fprintf(stderr, "FATAL ERROR: %s\n", strerror(ENOMEM));
  exit(NBFC_EXIT_FATAL);
}

void* Mem_Malloc(const size_t size) {
  void* p = malloc(size);
  if (unlikely(!p))
    Mem_FatalError();
  return p;
}

void* Mem_Calloc(const size_t nmemb, const size_t size) {
  void* p = calloc(nmemb, size);
  if (unlikely(!p))
    Mem_FatalError();
  return p;
}

void* Mem_Realloc(void* p, const size_t size) {
  void* new_p = realloc(p, size);
  if (unlikely(! new_p))
    Mem_FatalError();
  return new_p;
}

char* Mem_Strdup(const char* s) {
  char* p = strdup(s);
  if (unlikely(!p))
    Mem_FatalError();
  return p;
}

void Mem_Free(void* p) {
  free(p);
}
