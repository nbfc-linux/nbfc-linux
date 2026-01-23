#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include <stddef.h>    // size_t
#include <fcntl.h>     // mode_t
#include <sys/types.h> // ssize_t
#include <unistd.h>    // access, F_OK, R_OK
#include <stdbool.h>

ssize_t slurp_file(char*, ssize_t, const char*);
ssize_t slurp_file_dynamic(char**, const char*);
ssize_t write_file(const char*, int, mode_t, const char*, ssize_t);

static inline bool file_exists(const char* path) {
  return (access(path, F_OK) == 0);
}

static inline bool file_is_readable(const char* path) {
  return (access(path, R_OK) == 0);
}

#endif
