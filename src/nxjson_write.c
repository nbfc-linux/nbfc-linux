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
  bool first_write;
  int indent;
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

static void nx_json_write_string_escaped(NX_JSON_Write* obj, const char* s) {
  char buf[64];

  while (*s) {
    unsigned char c = (unsigned char)*s++;

    switch (c) {
      case '\"': _nx_json_write(obj, "\\\""); break;
      case '\\': _nx_json_write(obj, "\\\\"); break;
      case '\b': _nx_json_write(obj, "\\b"); break;
      case '\f': _nx_json_write(obj, "\\f"); break;
      case '\n': _nx_json_write(obj, "\\n"); break;
      case '\r': _nx_json_write(obj, "\\r"); break;
      case '\t': _nx_json_write(obj, "\\t"); break;

      default:
        if (c < 0x20) {
          snprintf(buf, sizeof(buf), "\\u%04x", c);
          _nx_json_write(obj, buf);
        }
        else if (c < 0x80) {
          buf[0] = c;
          buf[1] = '\0';
          _nx_json_write(obj, buf);
        }
        else {
          unsigned codepoint;

          if ((c & 0xE0) == 0xC0) {
            codepoint =
              ((c & 0x1F) << 6) |
              (*s++ & 0x3F);
          }
          else if ((c & 0xF0) == 0xE0) {
            codepoint =
              ((c & 0x0F) << 12) |
              ((*s++ & 0x3F) << 6) |
              (*s++ & 0x3F);
          }
          else if ((c & 0xF8) == 0xF0) {
            codepoint =
              ((c & 0x07) << 18) |
              ((*s++ & 0x3F) << 12) |
              ((*s++ & 0x3F) << 6) |
              (*s++ & 0x3F);
          }
          else {
            _nx_json_write(obj, "\\uFFFD");
            continue;
          }

          if (codepoint <= 0xFFFF) {
            snprintf(buf, sizeof(buf), "\\u%04X", codepoint);
            _nx_json_write(obj, buf);
          }
          else {
            codepoint -= 0x10000;
            unsigned high = 0xD800 | (codepoint >> 10);
            unsigned low = 0xDC00 | (codepoint & 0x3FF);

            snprintf(buf, sizeof(buf), "\\u%04X\\u%04X", high, low);
            _nx_json_write(obj, buf);
          }
        }
    }
  }
}

static void nx_json_write_indention(NX_JSON_Write* obj, int indent) {
  if (! obj->first_write)
    _nx_json_write(obj, "\n");

  obj->first_write = false;

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
      nx_json_write(obj, nx->val.children.first, indent + obj->indent);
      nx_json_write_indention(obj, indent);
      _nx_json_write(obj, "}");
      break;
    case NX_JSON_ARRAY:
      nx_json_write_indention(obj, indent);
      nx_json_write_key_not_null(obj, nx);
      _nx_json_write(obj, "[");
      nx_json_write(obj, nx->val.children.first, indent + obj->indent);
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

bool nxjson_send_to_fd(const nx_json* json, int fd, int indent) {
  struct NX_JSON_Write write_obj;
  write_obj.fd = fd;
  write_obj.mode = WriteMode_Send;
  write_obj.success = true;
  write_obj.first_write = true;
  write_obj.indent = indent;

  nx_json_write(&write_obj, json, 0);

  return write_obj.success;
}

bool nxjson_write_to_fd(const nx_json* json, int fd, int indent) {
  struct NX_JSON_Write write_obj;
  write_obj.fd = fd;
  write_obj.mode = WriteMode_Write;
  write_obj.success = true;
  write_obj.first_write = true;
  write_obj.indent = indent;

  nx_json_write(&write_obj, json, 0);

  return write_obj.success;
}
