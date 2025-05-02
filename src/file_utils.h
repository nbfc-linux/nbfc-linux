#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include <stddef.h>    // size_t
#include <fcntl.h>     // mode_t
#include <sys/types.h> // ssize_t

ssize_t slurp_file(char*, ssize_t, const char*);
ssize_t write_file(const char*, int, mode_t, const char*, ssize_t);

#endif
