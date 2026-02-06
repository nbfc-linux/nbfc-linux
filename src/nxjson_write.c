#include "nxjson_write.h"

#include "send.h"

#include <string.h> // strlen
#include <unistd.h> // write

enum NX_JSON_WriteMode {
  WriteMode_Send,
  WriteMode_Write
};

struct NX_JSON_Write {
  int fd;
  enum NX_JSON_WriteMode mode;
  bool success;
};
typedef struct NX_JSON_Write NX_JSON_Write;

static void _nx_json_write(NX_JSON_Write* obj, const char* s) {
  const size_t len = strlen(s);

  if (obj->mode == WriteMode_Write) {
    write(obj->fd, s, len);
  }
  else {
    obj->success &= Send(obj->fd, s, len);
  }
}

// ", \, and control codes (anything less than U+0020).
static void nx_json_write_string_escaped(NX_JSON_Write* obj, const char* s) {
  char buf[16];

  for (; *s; ++s) {
    if (*s == '"' || *s == '\\') {
      buf[0] = '\\';
      buf[1] = *s;
      buf[2] = '\0';
    }
    else if (*s < 0x20) {
      snprintf(buf, sizeof(buf), "\\u%.4X", *s);
    }
    else {
      buf[0] = *s;
      buf[1] = '\0';
    }

    _nx_json_write(obj, buf);
  }
}

static void nx_json_write_indention(NX_JSON_Write* obj, int indent) {
  _nx_json_write(obj, "\n");
  for (int i = 0; i < indent; ++i)
    _nx_json_write(obj, " ");
}

static void nx_json_write_key_not_null(NX_JSON_Write* obj, const nx_json* nx) {
  if (nx->key != NULL) {
    _nx_json_write(obj, "\"");
    _nx_json_write(obj, nx->key);
    _nx_json_write(obj, "\": ");
  }
}

void nx_json_write(NX_JSON_Write* obj, const nx_json *nx, int indent) {
  static char buf[32];

  while (nx != NULL) {
    switch (nx->type) {
    case NX_JSON_OBJECT:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      _nx_json_write(obj, "{");
      nx_json_write(obj, nx->val.children.first, indent + 3);
      nx_json_write_indention(obj, indent);
      _nx_json_write(obj, "}");
      break;
    case NX_JSON_ARRAY:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      _nx_json_write(obj, "[");
      nx_json_write(obj, nx->val.children.first, indent + 3);
      nx_json_write_indention(obj, indent);
      _nx_json_write(obj, "]");
      break;
    case NX_JSON_STRING:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      _nx_json_write(obj, "\"");
      nx_json_write_string_escaped(obj, nx->val.text);
      _nx_json_write(obj, "\"");
      break;
    case NX_JSON_BOOL:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      _nx_json_write(obj, nx->val.u ? "true" : "false");
      break;
    case NX_JSON_INTEGER:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      snprintf(buf, sizeof(buf), "%ld", nx->val.i);
      _nx_json_write(obj, buf);
      break;
    case NX_JSON_DOUBLE:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      snprintf(buf, sizeof(buf), "%lf", nx->val.dbl);
      _nx_json_write(obj, buf);
      break;
    case NX_JSON_NULL:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      _nx_json_write(obj, "null");
      break;
    }

    nx = nx->next;
    if (nx != NULL)
      _nx_json_write(obj, ",");
  }
}

bool nxjson_send_to_fd(const nx_json* json, int fd) {
  struct NX_JSON_Write write_obj;
  write_obj.fd = fd;
  write_obj.mode = WriteMode_Send;
  write_obj.success = true;

  nx_json_write(&write_obj, json, 0);

  return write_obj.success;
}

bool nxjson_write_to_fd(const nx_json* json, int fd) {
  struct NX_JSON_Write write_obj;
  write_obj.fd = fd;
  write_obj.mode = WriteMode_Write;
  write_obj.success = true;

  nx_json_write(&write_obj, json, 0);

  return write_obj.success;
}
