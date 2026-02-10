#include "nxjson_memory.h"

#include "memory.h"
#include "nxjson.h"

#include <string.h> // memset
#include <stdbool.h>

static nx_json NXJSON_OBJECTS[NXJSON_MEMORY_NUM_OBJECTS] = {0};
static bool    NXJSON_OBJECTS_USED[NXJSON_MEMORY_NUM_OBJECTS] = {0};

void* NXJSON_Memory_Calloc(size_t nmemb, size_t size) {
  for (int i = 0; i < NXJSON_MEMORY_NUM_OBJECTS; ++i) {
    if (! NXJSON_OBJECTS_USED[i]) {
      NXJSON_OBJECTS_USED[i] = true;
      nx_json* obj = &NXJSON_OBJECTS[i];
      memset(obj, 0, sizeof(nx_json));
      return (void*) obj;
    }
  }

  return Mem_Calloc(nmemb, size);
}

void NXJSON_Memory_Free(void* p) {
  nx_json* obj = (nx_json*) p;

  if (obj >= NXJSON_OBJECTS && obj < NXJSON_OBJECTS + NXJSON_MEMORY_NUM_OBJECTS) {
    NXJSON_OBJECTS_USED[obj - NXJSON_OBJECTS] = false;
    return;
  }

  Mem_Free(p);
}
