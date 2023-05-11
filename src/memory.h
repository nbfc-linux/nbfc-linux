#ifndef NBFC_MEMORY_H_
#define NBFC_MEMORY_H_

#include <stddef.h>

void* Mem_Malloc(size_t);
void* Mem_Calloc(size_t, size_t);
void* Mem_Realloc(void*, size_t);
char* Mem_Strdup(const char*);
void  Mem_Free(void*);

#endif
