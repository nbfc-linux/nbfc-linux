#ifndef NXJSON_MEMORY_H_
#define NXJSON_MEMORY_H_

#define NXJSON_MEMORY_NUM_OBJECTS 256

void* NXJSON_Memory_Calloc(size_t, size_t);
void  NXJSON_Memory_Free(void*);

#endif
