#ifndef NBFC_MEMORY_H_
#define NBFC_MEMORY_H_

#include "macros.h"

#include <stddef.h>

#define MEMORY_TRACE_ALLOCS 0

#define MEMORY_TRACE __FILE__ ":" STRINGIFY(__LINE__)

#if MEMORY_TRACE_ALLOCS

void* Memory_Malloc(const char* trace, size_t);
void* Memory_Calloc(const char* trace, size_t, size_t);
void* Memory_Realloc(const char* trace, void*, size_t);
char* Memory_Strdup(const char* trace, const char*);
void  Memory_Free(void*);

#define Mem_Malloc(SIZE)        Memory_Malloc(MEMORY_TRACE, SIZE)
#define Mem_Calloc(NMEMB, SIZE) Memory_Calloc(MEMORY_TRACE, NMEMB, SIZE)
#define Mem_Realloc(PTR, SIZE)  Memory_Realloc(MEMORY_TRACE, PTR, SIZE)
#define Mem_Strdup(PTR)         Memory_Strdup(MEMORY_TRACE, PTR)
#define Mem_Free(PTR)           Memory_Free(PTR)

#else

void* Memory_Malloc(size_t);
void* Memory_Calloc(size_t, size_t);
void* Memory_Realloc(void*, size_t);
char* Memory_Strdup(const char*);
void  Memory_Free(void*);

#define Mem_Malloc(SIZE)        Memory_Malloc(SIZE)
#define Mem_Calloc(NMEMB, SIZE) Memory_Calloc(NMEMB, SIZE)
#define Mem_Realloc(PTR, SIZE)  Memory_Realloc(PTR, SIZE)
#define Mem_Strdup(PTR)         Memory_Strdup(PTR)
#define Mem_Free(PTR)           Memory_Free(PTR)

#endif

#endif
