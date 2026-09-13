#include "nxjson_write.h"

#include "macros.h"
#include "send.h"

#include <string.h> // strlen
#include <unistd.h> // write

typedef enum NBFC_PACKED_ENUM {
  NxJson_WriteModeSend,
  NxJson_WriteModeWrite
} NxJson_WriteMode;

typedef struct {
  int fd;
  NxJson_WriteMode mode;
  bool success;
  bool first_write;
  int indent;
} NxJson_Writer;

static void NxJson_WriteRaw(NxJson_Writer* obj, const char* s) {
  const size_t len = strlen(s);

  if (obj->mode == NxJson_WriteModeWrite) {
    write(obj->fd, s, len);
  }
  else {
    obj->success &= Send(obj->fd, s, len);
  }
}

static void NxJson_WriteStringEscaped(NxJson_Writer* obj, const char* s) {
  char buf[64];

  while (*s) {
    unsigned char c = (unsigned char)*s++;

    switch (c) {
    case '\"': NxJson_WriteRaw(obj, "\\\""); break;
    case '\\': NxJson_WriteRaw(obj, "\\\\"); break;
    case '\b': NxJson_WriteRaw(obj, "\\b"); break;
    case '\f': NxJson_WriteRaw(obj, "\\f"); break;
    case '\n': NxJson_WriteRaw(obj, "\\n"); break;
    case '\r': NxJson_WriteRaw(obj, "\\r"); break;
    case '\t': NxJson_WriteRaw(obj, "\\t"); break;

    default:
      if (c < 0x20) {
        snprintf(buf, sizeof(buf), "\\u%04x", c);
        NxJson_WriteRaw(obj, buf);
      }
      else if (c < 0x80) {
        buf[0] = c;
        buf[1] = '\0';
        NxJson_WriteRaw(obj, buf);
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
          NxJson_WriteRaw(obj, "\\uFFFD");
          continue;
        }

        if (codepoint <= 0xFFFF) {
          snprintf(buf, sizeof(buf), "\\u%04X", codepoint);
          NxJson_WriteRaw(obj, buf);
        }
        else {
          codepoint -= 0x10000;
          unsigned high = 0xD800 | (codepoint >> 10);
          unsigned low = 0xDC00 | (codepoint & 0x3FF);

          snprintf(buf, sizeof(buf), "\\u%04X\\u%04X", high, low);
          NxJson_WriteRaw(obj, buf);
        }
      }
    }
  }
}

static void NxJson_WriteIndention(NxJson_Writer* obj, int indent) {
  if (!obj->first_write)
    NxJson_WriteRaw(obj, "\n");

  obj->first_write = false;

  for (int i = 0; i < indent; ++i)
    NxJson_WriteRaw(obj, " ");
}

static void NxJson_WriteKeyNotNull(NxJson_Writer* obj, const nx_json* json) {
  if (json->key != NULL) {
    NxJson_WriteRaw(obj, "\"");
    NxJson_WriteRaw(obj, json->key);
    NxJson_WriteRaw(obj, "\": ");
  }
}

void NxJson_WriteJsonObject(NxJson_Writer* obj, const nx_json* json, int indent) {
  static char buf[32];

  while (json != NULL) {
    switch (json->type) {
    case NX_JSON_OBJECT:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      NxJson_WriteRaw(obj, "{");
      NxJson_WriteJsonObject(obj, json->val.children.first, indent + obj->indent);
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteRaw(obj, "}");
      break;
    case NX_JSON_ARRAY:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      NxJson_WriteRaw(obj, "[");
      NxJson_WriteJsonObject(obj, json->val.children.first, indent + obj->indent);
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteRaw(obj, "]");
      break;
    case NX_JSON_STRING:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      NxJson_WriteRaw(obj, "\"");
      NxJson_WriteStringEscaped(obj, json->val.text);
      NxJson_WriteRaw(obj, "\"");
      break;
    case NX_JSON_BOOL:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      NxJson_WriteRaw(obj, json->val.u ? "true" : "false");
      break;
    case NX_JSON_INTEGER:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      snprintf(buf, sizeof(buf), "%ld", json->val.i);
      NxJson_WriteRaw(obj, buf);
      break;
    case NX_JSON_DOUBLE:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      snprintf(buf, sizeof(buf), "%lf", json->val.dbl);
      NxJson_WriteRaw(obj, buf);
      break;
    case NX_JSON_NULL:
      NxJson_WriteIndention(obj, indent);
      NxJson_WriteKeyNotNull(obj, json);
      NxJson_WriteRaw(obj, "null");
      break;
    }

    json = json->next;
    if (json != NULL)
      NxJson_WriteRaw(obj, ",");
  }
}

bool nxjson_send_to_fd(const nx_json* json, int fd, int indent) {
  NxJson_Writer writer;
  writer.fd = fd;
  writer.mode = NxJson_WriteModeSend;
  writer.success = true;
  writer.first_write = true;
  writer.indent = indent;

  NxJson_WriteJsonObject(&writer, json, 0);

  return writer.success;
}

bool nxjson_write_to_fd(const nx_json* json, int fd, int indent) {
  NxJson_Writer writer;
  writer.fd = fd;
  writer.mode = NxJson_WriteModeWrite;
  writer.success = true;
  writer.first_write = true;
  writer.indent = indent;

  NxJson_WriteJsonObject(&writer, json, 0);

  return writer.success;
}
