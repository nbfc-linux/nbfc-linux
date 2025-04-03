#ifndef NBFC_SLURP_FILE_H
#define NBFC_SLURP_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

/// Read `file` into `buf`
static inline ssize_t slurp_file(char* buf, size_t size, const char* file) {
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

static inline int write_file(const char* file, int flags, mode_t mode, const char* content, size_t size) {
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

#endif
