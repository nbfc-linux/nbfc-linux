#include "file_utils.h"

#include "memory.h"

#include <errno.h>  // errno
#include <unistd.h> // open, read, write, close
#include <string.h> // memcpy

/*
 * Read a file into a buffer.
 *
 * The file is read into `buf` with a maximum length of `size` bytes,
 * including the terminating null byte.
 *
 * Returns a `FileResult`:
 * - FileResult.ok: True on success, false otherwise.
 * - FileResult.len: The length of the string, excluding null byte.
 *
 * The function fails, and sets `errno` if:
 *  - the file did not fit into the buffer (EFBIG)
 *  - the file could not be opened (see open(2))
 *  - the file could not be read (see read(2))
 */
FileResult File_Read(char* buf, size_t size, const char* file) {
  FileResult result;
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

/*
 * Read a file into a dynamically allocated buffer.
 *
 * The buffer is allocated by the function and stored in `*out`.
 *
 * The caller is responsible for freeing the buffer with `free()`.
 *
 * Returns a `FileResult`:
 * - FileResult.ok: True on success, false otherwise.
 * - FileResult.len: The length of the string, excluding the null byte.
 *
 * The function fails and sets `errno` if:
 * - the file could not be opened (see open(2))
 * - the file could not be read (see read(2))
 */
FileResult File_ReadDynamic(char** out, const char* file) {
  FileResult result;
  result.ok = true;
  result.len = 0;

  const int fd = open(file, O_RDONLY);
  if (fd < 0) {
    result.ok = false;
    return result;
  }

  char buf[4096];
  *out = Mem_Calloc(1, 1);

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

/*
 * Write data to a file.
 *
 * The file is opened using `flags` and `mode`, and `size` bytes from `content`
 * are written to it.
 *
 * Returns a `FileResult`:
 * - FileResult.ok: True on success, false otherwise.
 * - FileResult.len: The number of bytes written.
 *
 * The function fails and sets `errno` if:
 * - the file could not be opened (see open(2))
 * - the file could not be written (see write(2))
 */
FileResult File_Write(const char* file, int flags, mode_t mode, const char* content, size_t size) {
  FileResult result;
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
