#include "file_utils.h"

#include <errno.h>
#include <unistd.h>

ssize_t slurp_file(char* buf, ssize_t size, const char* file) {
  ssize_t   nread = -1;
  const int fd = open(file, O_RDONLY);
  if (fd >= 0) {
    nread = read(fd, buf, size);
    if (nread == size) {
      errno = EFBIG;
      nread = -1;
    }
    else if (nread >= 0)
      buf[nread] = '\0';

    int old_errno = errno;
    close(fd);
    errno = old_errno;
  }
  return nread;
}

ssize_t write_file(const char* file, int flags, mode_t mode, const char* content, ssize_t size) {
  const int fd = open(file, flags, mode);
  if (fd == -1)
    return -1;

  ssize_t nwritten = write(fd, content, size);

  int old_errno = errno;
  close(fd);
  errno = old_errno;

  if (nwritten == -1)
    return -1;

  return nwritten;
}
