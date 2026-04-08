#ifndef NBFC_BUFFER_H_
#define NBFC_BUFFER_H_

#include <stddef.h>

char* Buffer_Get(size_t);
void  Buffer_Release(char*, size_t);

#endif
