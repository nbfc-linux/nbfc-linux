#ifndef NBFC_FILE_UTILS_H_
#define NBFC_FILE_UTILS_H_

#include <stddef.h> // size_t
#include <fcntl.h>  // mode_t
#include <unistd.h> // access, F_OK, R_OK
#include <stdbool.h>

struct file_op_result {
  bool ok;
  size_t len;
};
typedef struct file_op_result file_op_result;

file_op_result slurp_file(char*, size_t, const char*);
file_op_result slurp_file_dynamic(char**, const char*);
file_op_result write_file(const char*, int, mode_t, const char*, size_t);

static inline bool file_exists(const char* path) {
  return (access(path, F_OK) == 0);
}

static inline bool file_is_readable(const char* path) {
  return (access(path, R_OK) == 0);
}

#endif
