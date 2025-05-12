#include "protocol.h"

#include "nbfc.h"
#include "memory.h"
#include "nxjson_utils.h"
#include "reverse_nxjson.h"

#include <unistd.h>
#include <sys/socket.h>

Error* Protocol_Send(int socket, const char* buffer, size_t length) {
  size_t total_sent = 0;

  while (total_sent < length) {
    size_t to_send = length - total_sent;

    if (to_send > PROTOCOL_BUFFER_SIZE)
      to_send = PROTOCOL_BUFFER_SIZE;

    int ret = send(socket, buffer + total_sent, to_send, MSG_NOSIGNAL);
    if (ret < 0) {
      if (errno != EINTR && errno != EAGAIN)
        return err_stdlib(0, "send()");
      else
        continue;
    }

    total_sent += ret;
  }

  return err_success();
}

Error* Protocol_Send_Json(int socket, const nx_json* json) {
  Error* e;
  char buf[NBFC_MAX_FILE_SIZE];
  StringBuf s = { buf, 0, sizeof(buf) };
  buf[0] = '\0';
  nx_json_to_string(json, &s, 0);
  // TODO: handle case if buffer is too small

  e = Protocol_Send(socket, s.s, s.size);
  if (e)
    return e;

  e = Protocol_Send_End(socket);
  if (e)
    return e;

  return err_success();
}

Error *Protocol_Receive_Json(int socket, char** buf, const nx_json** out) {
  char buffer[PROTOCOL_BUFFER_SIZE] = {0};
  int nread;
  const nx_json* json = NULL;

  char* msg = NULL;
  int msg_size = 0;

  while ((nread = read(socket, buffer, PROTOCOL_BUFFER_SIZE)) > 0) {
    msg = Mem_Realloc(msg, msg_size + nread + 1);
    memcpy(msg + msg_size, buffer, nread);
    msg_size += nread;
    msg[msg_size] = '\0';

    char *end_marker_pos = strstr(msg, PROTOCOL_END_MARKER);
    if (end_marker_pos != NULL) {
      *end_marker_pos = '\0';
      break;
    }
  }

  if (msg) {
    json = nx_json_parse_utf8(msg);
    if (! json) {
      Mem_Free(msg);
      return err_nxjson(0, "Invalid JSON");
    }

    *buf = msg;
    *out = json;
  }
  else
    return err_string(0, "Empty response");

  return err_success();
}

Error* Protocol_Send_Error(int socket, const char* message) {
  nx_json error  = {0};
  error.type     = NX_JSON_STRING;
  error.key      = "Error";
  error.val.text = message;

  nx_json obj             = {0};
  obj.type                = NX_JSON_OBJECT;
  obj.val.children.length = 1;
  obj.val.children.first  = &error;
  obj.val.children.last   = &error;

  return Protocol_Send_Json(socket, &obj);
}
