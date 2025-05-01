#ifndef STACK_MEMORY_H_
#define STACK_MEMORY_H_

#include <stddef.h>

struct StackMemory {
    void* start;
    void* current;
    size_t size;
};
typedef struct StackMemory StackMemory;

extern StackMemory StackMemory_Memory;

static inline void StackMemory_Init(void* buf, size_t size) {
  StackMemory_Memory.start = buf;
  StackMemory_Memory.current = buf;
  StackMemory_Memory.size = size;
}

static inline void StackMemory_Destroy() {
  StackMemory_Memory.start = NULL;
}

#endif
