#include "file_utils.h"

#include "memory.h"

#include <errno.h>  // errno
#include <unistd.h> // open, read, write, close
#include <string.h> // memcpy

ssize_t slurp_file(char* buf, ssize_t size, const char* file) {
  const int fd = open(file, O_RDONLY);
  if (fd < 0)
    return -1;

  ssize_t nread = read(fd, buf, size);

  if (nread == size) {
    errno = EFBIG;
    nread = -1;
  }
  else if (nread >= 0)
    buf[nread] = '\0';

  int old_errno = errno;
  close(fd);
  errno = old_errno;

  return nread;
}

ssize_t slurp_file_dynamic(char** out, const char* file) {
  const int fd = open(file, O_RDONLY);
  if (fd < 0)
    return -1;

  char buf[4096];
  ssize_t nread;
  ssize_t size = 0;
  *out = NULL;

  while ((nread = read(fd, buf, sizeof(buf))) > 0) {
    *out = Mem_Realloc(*out, size + nread + 1);
    memcpy(*out + size, buf, nread);
    size += nread;
  }

  int errno_save = errno;
  close(fd);
  errno = errno_save;

  if (nread < 0) {
    Mem_Free(*out);
    *out = NULL;
    return -1;
  }

  if (*out)
    (*out)[size] = '\0';

  return size;
}

ssize_t write_file(const char* file, int flags, mode_t mode, const char* content, ssize_t size) {
  const int fd = open(file, flags, mode);
  if (fd == -1)
    return -1;

  ssize_t nwritten = write(fd, content, size);

  int old_errno = errno;
  close(fd);
  errno = old_errno;

  return nwritten;
}
