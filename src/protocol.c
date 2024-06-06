#include "protocol.h"

#include "nxjson_utils.h"
#include "reverse_nxjson.h"

#define PROTOCOL_BUFFER_SIZE 4096

Error* Protocol_Send(int socket, const void* buffer, size_t length, int flags) {
  int ret = send(socket, buffer, length, flags | MSG_NOSIGNAL);
  if (ret < 0)
    return err_stdlib(0, "send");
  return err_success();
}

Error* Protocol_Send_Json(int socket, const nx_json* json) {
  Error* e;
  char buf[NBFC_MAX_FILE_SIZE];
  StringBuf s = { buf, 0, sizeof(buf) };
  buf[0] = '\0';
  nx_json_to_string(json, &s, 0);
  // TODO: handle case if buffer is too small

  e = Protocol_Send(socket, s.s, s.size, 0);
  if (e)
    return e;

  e = Protocol_Send_End(socket);
  if (e)
    return e;

  return err_success();
}

Error *Protocol_Receive_Json(int socket, char** buf, const nx_json** out) {
  char buffer[PROTOCOL_BUFFER_SIZE] = {0};
  int valread;
  const nx_json* json = NULL;

  char* msg = NULL;
  int msg_size = 0;

  // Empfang von Daten
  while ((valread = read(socket, buffer, PROTOCOL_BUFFER_SIZE)) > 0) {
    msg = realloc(msg, msg_size + valread + 1);
    memcpy(msg + msg_size, buffer, valread);
    msg_size += valread;
    msg[msg_size] = '\0';

    char *end_marker_pos = strstr(msg, PROTOCOL_END_MARKER);
    if (end_marker_pos != NULL) {
      *end_marker_pos = '\0';
      break;
    }

    // Puffer leeren
    memset(buffer, 0, PROTOCOL_BUFFER_SIZE);
  }

  if (msg) {
    json = nx_json_parse_utf8(msg);
    if (! json) {
      free(msg);
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
  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);
  create_json_string("error", o, message);
  Error* e = Protocol_Send_Json(socket, o);
  nx_json_free(o);
  return e;
}

