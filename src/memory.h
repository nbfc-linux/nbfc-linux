#ifndef NBFC_MEMORY_H_
#define NBFC_MEMORY_H_

#include <stddef.h>

typedef struct Memory_Pool Memory_Pool;
struct Memory_Pool {
  size_t bufsize;
  char*  buffer;
  size_t size;
};

typedef struct Memory Memory;
struct Memory {
  Memory_Pool pools[2];
  size_t      index;
};

extern Memory MemoryMain;
extern Memory MemoryTemp;

void  Memory_Reset(Memory*);
void  Memory_AddPool(Memory*, char*, size_t);
void* Memory_Malloc(Memory*, size_t, size_t);
void* Memory_Calloc(Memory*, size_t, size_t);
char* Memory_Strdup(Memory*, const char*);

#define Mem_AddPool(BUF, SIZE)  Memory_AddPool(&MemoryMain, BUF, SIZE)
#define Mem_Malloc(NMEMB, SIZE) Memory_Malloc(&MemoryMain, NMEMB, SIZE)
#define Mem_Calloc(NMEMB, SIZE) Memory_Calloc(&MemoryMain, NMEMB, SIZE)
#define Mem_Strdup(CHARS)       Memory_Strdup(&MemoryMain, CHARS)

#define Temp_Reset()             Memory_Reset(&MemoryTemp)
#define Temp_AddPool(BUF, SIZE)  Memory_AddPool(&MemoryTemp, BUF, SIZE)
#define Temp_Malloc(NMEMB, SIZE) Memory_Malloc(&MemoryTemp, NMEMB, SIZE)
#define Temp_Calloc(NMEMB, SIZE) Memory_Calloc(&MemoryTemp, NMEMB, SIZE)
#define Temp_Strdup(CHARS)       Memory_Strdup(&MemoryTemp, CHARS)

#endif
