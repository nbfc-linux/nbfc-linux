#include "memory.h"

#include "macros.h" // ARRAY_SSIZE
#include "nbfc.h"

#include <errno.h>  // errno
#include <stdlib.h> // malloc
#include <string.h> // memcpy, strerror

Memory MemoryMain;
Memory MemoryTemp;

static inline void Memory_FatalError(const char* cause) {
  fprintf(stderr, "%s: %s\n", cause, strerror(errno));
  exit(NBFC_EXIT_FATAL);
}

void Memory_AddPool(Memory* mem, char* buffer, size_t size) {
  mem->pools[mem->index].buffer  = buffer;
  mem->pools[mem->index].size    = size;
  mem->pools[mem->index].bufsize = size;
  if (++(mem->index) == ARRAY_SSIZE(mem->pools))
    mem->index = 0;
}

static inline size_t Memory_Pad(char* buf, size_t size) {
  const size_t max_pad = size <= 8 ? size : 8;
  const size_t overstanding_bytes = ((uintptr_t)buf) % max_pad;
  const size_t pad = overstanding_bytes ? max_pad - overstanding_bytes : 0;
  return pad;
}

static void* Memory_TryAllocFromPool(Memory* memory, size_t nmemb, size_t size) {
  for (range(int, i, 0, ARRAY_SSIZE(memory->pools))) {
    Memory_Pool* pool = &memory->pools[i];
    if (! pool->buffer)
      continue;

    const size_t pad = Memory_Pad(pool->buffer, size);

    if (pad <= pool->size && size * nmemb + pad <= pool->size) {
      void* p = pool->buffer + pad;
      pool->size   -= pad + size * nmemb;
      pool->buffer += pad + size * nmemb;
      return p;
    }
  }

  return NULL;
}

void Memory_Reset(Memory* memory) {
  for (int i = 0; i < ARRAY_SSIZE(memory->pools); ++i) {
    Memory_Pool* pool = &memory->pools[i];
    pool->buffer = pool->buffer + pool->bufsize - pool->size;
    pool->size = pool->bufsize;
  }
}

void* Memory_Malloc(Memory* memory, size_t nmemb, size_t size) {
  void* p = Memory_TryAllocFromPool(memory, nmemb, size);
  if (!p && !(p = malloc(nmemb * size)))
    Memory_FatalError("Memory_Malloc");
  return p;
}

void* Memory_Calloc(Memory* memory, const size_t nmemb, const size_t size) {
  void* p = Memory_TryAllocFromPool(memory, nmemb, size);
  if (!p && !(p = malloc(nmemb * size)))
    Memory_FatalError("Memory_Calloc");

  memset(p, 0, nmemb * size);
  return p;
}

char* Memory_Strdup(Memory* memory, const char* s) {
  const size_t len = strlen(s) + 1;

  void* p = Memory_TryAllocFromPool(memory, len, 1);
  if (!p && !(p = malloc(len)))
    Memory_FatalError("Memory_Strdup");

  memcpy(p, s, len);
  return (char*) p;
}
