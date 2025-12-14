#include "memory.h"

#include "macros.h" // unlikely
#include "nbfc.h"   // NBFC_EXIT_FATAL

#include <stdlib.h> // malloc, calloc, realloc, free, exit
#include <string.h> // strlen, strcpy, strerror
#include <stdio.h>  // fprintf
#include <errno.h>  // ENOMEM

static void Mem_FatalError() {
  fprintf(stderr, "FATAL ERROR: %s\n", strerror(ENOMEM));
  exit(NBFC_EXIT_FATAL);
}

void* Mem_Malloc(const size_t size) {
  void* p = calloc(1, size);
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
  const size_t len = strlen(s);
  char* p = Mem_Malloc(len + 1);
  strcpy(p, s);
  return p;
}

void Mem_Free(void* p) {
  free(p);
}
