#ifndef NBFC_PROTOCOL_H_
#define NBFC_PROTOCOL_H_

#include "error.h"
#include "nxjson.h"
#include "send.h"

#define PROTOCOL_END_MARKER     "\nEND"
#define PROTOCOL_END_MARKER_LEN 4
#define PROTOCOL_BUFFER_SIZE    4096

Error Protocol_SendJson(int, const nx_json*);
Error Protocol_SendError(int, const char*);
Error Protocol_ReceiveJson(int, char**, const nx_json**);

static inline Error Protocol_Send(int socket, const char* s, size_t len) {
  if (! Send(socket, s, len))
    return err_stdlib("send()");
  return err_success();
}

static inline Error Protocol_SendEnd(int socket) {
  return Protocol_Send(socket, PROTOCOL_END_MARKER, PROTOCOL_END_MARKER_LEN);
}

#endif
