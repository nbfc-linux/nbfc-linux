#include "io_utils.h"

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#define SEND_BUFFER_SIZE  4096
#define WRITE_BUFFER_SIZE 4096

bool IO_SendAll(int socket, const char* buffer, size_t length) {
  size_t total_sent = 0;

  while (total_sent < length) {
    size_t to_send = length - total_sent;

    if (to_send > SEND_BUFFER_SIZE)
      to_send = SEND_BUFFER_SIZE;

    ssize_t ret = send(socket, buffer + total_sent, to_send, MSG_NOSIGNAL);
    if (ret < 0) {
      if (errno != EINTR && errno != EAGAIN)
        return false;
      else
        continue;
    }

    total_sent += (size_t) ret;
  }

  return true;
}

bool IO_WriteAll(int fd, const char *buffer, size_t length)
{
  size_t total_written = 0;

  while (total_written < length) {
    size_t to_write = length - total_written;

    if (to_write > WRITE_BUFFER_SIZE)
      to_write = WRITE_BUFFER_SIZE;

    ssize_t ret = write(fd, buffer + total_written, to_write);

    if (ret < 0) {
      if (errno != EINTR && errno != EAGAIN)
        return false;

      continue;
    }

    total_written += (size_t) ret;
  }

  return true;
}
