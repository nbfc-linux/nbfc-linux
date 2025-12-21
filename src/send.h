#ifndef NBFC_SEND_H_
#define NBFC_SEND_H_

#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>

#define SEND_BUFFER_SIZE 4096

static bool Send(int socket, const char* buffer, size_t length) {
  size_t total_sent = 0;

  while (total_sent < length) {
    size_t to_send = length - total_sent;

    if (to_send > SEND_BUFFER_SIZE)
      to_send = SEND_BUFFER_SIZE;

    int ret = send(socket, buffer + total_sent, to_send, MSG_NOSIGNAL);
    if (ret < 0) {
      if (errno != EINTR && errno != EAGAIN)
        return false;
      else
        continue;
    }

    total_sent += ret;
  }

  return true;
}

#endif
