#include "file_utils.h"

#include "memory.h"

#include <errno.h>  // errno
#include <unistd.h> // open, read, write, close
#include <string.h> // memcpy

file_op_result slurp_file(char* buf, size_t size, const char* file) {
  file_op_result result;
  result.ok = true;
  result.len = 0;

  const int fd = open(file, O_RDONLY);
  if (fd < 0) {
    result.ok = false;
    return result;
  }

  while (result.len + 1 < size) {
    ssize_t nread = read(fd, buf + result.len, size - result.len - 1);

    if (nread < 0) {
      if (errno == EINTR)
        continue;

      result.ok = false;
      break;
    }

    if (nread == 0) {
      break;
    }

    result.len += (size_t) nread;
  }

  if (result.len + 1 >= size) {
    errno = EFBIG;
    result.ok = false;
  }
  else {
    buf[result.len] = '\0';
  }

  int old_errno = errno;
  close(fd);
  errno = old_errno;

  return result;
}

file_op_result slurp_file_dynamic(char** out, const char* file) {
  file_op_result result;
  result.ok = true;
  result.len = 0;

  const int fd = open(file, O_RDONLY);
  if (fd < 0) {
    result.ok = false;
    return result;
  }

  char buf[4096];
  *out = NULL;

  while (1) {
    ssize_t nread = read(fd, buf, sizeof(buf));

    if (nread < 0) {
      if (errno == EINTR)
        continue;

      result.ok = false;
      break;
    }

    if (nread == 0)
      break;

    *out = Mem_Realloc(*out, result.len + (size_t) nread + 1);
    memcpy(*out + result.len, buf, (size_t) nread);
    result.len += (size_t) nread;
  }

  int errno_save = errno;
  close(fd);
  errno = errno_save;

  if (! result.ok) {
    Mem_Free(*out);
    *out = NULL;
  }

  if (*out)
    (*out)[result.len] = '\0';

  return result;
}

file_op_result write_file(const char* file, int flags, mode_t mode, const char* content, size_t size) {
  file_op_result result;
  result.ok = true;
  result.len = 0;

  const int fd = open(file, flags, mode);
  if (fd == -1) {
    result.ok = false;
    return result;
  }

  while (result.len < size) {
    ssize_t nwritten = write(fd, content + result.len, size - result.len);

    if (nwritten < 0) {
      if (errno == EINTR)
        continue;

      result.ok = false;
      break;
    }

    if (nwritten == 0) {
      result.ok = false;
      errno = EIO;
      break;
    }

    result.len += (size_t) nwritten;
  }

  int old_errno = errno;
  close(fd);
  errno = old_errno;

  return result;
}
