#ifndef NBFC_BUFFER_H_
#define NBFC_BUFFER_H_

#include "nbfc.h"

#define BUFFER_SIZE NBFC_MAX_FILE_SIZE

char* Buffer_Get();
void  Buffer_Release(char*);

#endif
