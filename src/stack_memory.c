#include "stack_memory.h"

#include <stddef.h> // size_t
#include <stdint.h> // uintptr_t
#include <string.h> // memset

#include "memory.h"

#define ALIGNMENT sizeof(void*)

StackMemory StackMemory_Memory = {0};

static inline void* align_pointer(void* ptr) {
    uintptr_t p = (uintptr_t) ptr;
    p = (p + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
    return (void*) p;
}

void* StackMemory_Calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;

    if (total == 0)
        return NULL;

    if (! StackMemory_Memory.start)
      return Mem_Calloc(nmemb, size);

    void* aligned = align_pointer(StackMemory_Memory.current);
    uintptr_t offset = (uintptr_t) aligned - (uintptr_t) StackMemory_Memory.start;

    if (offset + total <= StackMemory_Memory.size) {
        void* result = aligned;
        StackMemory_Memory.current = (void*)((uintptr_t)aligned + total);
        memset(result, 0, total);
        return result;
    }

    return Mem_Calloc(nmemb, size);
}

void StackMemory_Free(void* ptr) {
  if (! ptr)
    return;

  if (! StackMemory_Memory.start) {
    Mem_Free(ptr);
    return;
  }

  uintptr_t mem_start = (uintptr_t) StackMemory_Memory.start;
  uintptr_t mem_end   = mem_start + StackMemory_Memory.size;
  uintptr_t p         = (uintptr_t) ptr;

  if (p < mem_start || p >= mem_end) {
    Mem_Free(ptr);
  }
}
