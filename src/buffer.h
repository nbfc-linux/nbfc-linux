#ifndef NBFC_BUFFER_H_
#define NBFC_BUFFER_H_

#include "nbfc.h"

#include <sys/types.h>

char* Buffer_Get(size_t);
void  Buffer_Release(char*, size_t);

#endif
