#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <sys/socket.h>

#define PROTOCOL_END_MARKER     "\nEND"
#define PROTOCOL_END_MARKER_LEN 4

Error* Protocol_Send_Json(int, const nx_json*);
Error* Protocol_Receive_Json(int, char**, const nx_json**);
Error* Protocol_Send_Error(int, const char*);
Error* Protocol_Send(int socket, const void* buffer, size_t length, int flags);

static inline Error* Protocol_Send_End(int socket) {
  return Protocol_Send(socket, PROTOCOL_END_MARKER, PROTOCOL_END_MARKER_LEN, 0);
}

#endif
