#include "protocol.h"

#include "nbfc.h"
#include "memory.h"
#include "nxjson_utils.h"
#include "nxjson_write.h"

#include <unistd.h>
#include <sys/socket.h>

Error Protocol_Send_Json(int socket, const nx_json* json) {
  if (! nxjson_send_to_fd(json, socket))
    return err_stdlib("send()");

  return Protocol_Send_End(socket);
}

Error Protocol_Receive_Json(int socket, char** buf, const nx_json** out) {
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
      return err_nxjson("Invalid JSON");
    }

    *buf = msg;
    *out = json;
  }
  else
    return err_string("Empty response");

  return err_success();
}

Error Protocol_Send_Error(int socket, const char* message) {
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
