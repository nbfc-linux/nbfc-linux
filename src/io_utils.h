#ifndef NBFC_IO_UTILS_H_
#define NBFC_IO_UTILS_H_

#include <stddef.h> // size_t
#include <stdbool.h>

bool IO_SendAll(int, const char*, size_t);
bool IO_WriteAll(int, const char*, size_t);

#endif
