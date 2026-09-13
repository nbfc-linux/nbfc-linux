#ifndef NBFC_FILE_UTILS_H_
#define NBFC_FILE_UTILS_H_

#include <stddef.h> // size_t
#include <fcntl.h>  // mode_t
#include <unistd.h> // access, F_OK, R_OK
#include <stdbool.h>

struct FileResult {
  bool ok;
  size_t len;
};
typedef struct FileResult FileResult;

FileResult File_Read(char*, size_t, const char*);
FileResult File_ReadDynamic(char**, const char*);
FileResult File_Write(const char*, int, mode_t, const char*, size_t);

static inline bool File_Exists(const char* path) {
  return (access(path, F_OK) == 0);
}

static inline bool File_IsReadable(const char* path) {
  return (access(path, R_OK) == 0);
}

#endif
