#include "nxjson_to_string.h"

#include "nbfc.h"

#include <stdio.h>
#include <string.h>

#define StringBuf_ADD_INDENTION(S, N) \
    StringBuf_Printf(S, "\n%*s", N, "")

static const char* Json_EscapeString(char*, const size_t, const char*);

static void StringBuf_ADD_KEY_NOT_NULL(StringBuf* s, const nx_json* nx) {
  if (nx->key != NULL) {                                                       \
    StringBuf_AddCh(s, '"');                                                   \
    StringBuf_Printf(s, "%s", nx->key);                                        \
    StringBuf_Printf(s, "%s", "\": ");                                         \
  }
}

char *nx_json_to_string(const nx_json *nx, StringBuf* s, int indent) {
  static char buf[NBFC_MAX_FILE_SIZE];

  while (nx != NULL) {
    switch (nx->type) {
    case NX_JSON_OBJECT:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_AddCh(s, '{');
      nx_json_to_string(nx->val.children.first, s, indent + 3);
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_AddCh(s, '}');
      break;
    case NX_JSON_ARRAY:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_AddCh(s, '[');
      nx_json_to_string(nx->val.children.first, s, indent + 3);
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_AddCh(s, ']');
      break;
    case NX_JSON_STRING:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_AddCh(s, '"');
      Json_EscapeString(buf, sizeof(buf), nx->val.text);
      StringBuf_Printf(s, "%s", buf);
      StringBuf_AddCh(s, '"');
      break;
    case NX_JSON_BOOL:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_Printf(s, "%s", nx->val.u ? "true" : "false");
      break;
    case NX_JSON_INTEGER:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_Printf(s, "%ld", nx->val.i);
      break;
    case NX_JSON_DOUBLE:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_Printf(s, "%lf", nx->val.dbl);
      break;
    case NX_JSON_NULL:
      StringBuf_ADD_INDENTION(s, indent);
      StringBuf_ADD_KEY_NOT_NULL(s, nx);
      StringBuf_Printf(s, "%s", "null");
      break;
    }
    nx = nx->next;
    if (nx != NULL)
      StringBuf_AddCh(s, ',');
  }

  return s->s;
}

// ", \, and control codes (anything less than U+0020).
static const char* Json_EscapeString(char* buf, const size_t n, const char* s) {
  size_t i = 0;
  for (; *s && i < n - 6 - 1; ++s)
    if (*s == '"' || *s == '\\' || *s < 0x20) {
      snprintf(&buf[i], 7, "\\u%.4X", *s);
      i += 6;
    }
    else
      buf[i++] = *s;
  buf[i] = '\0';
  return buf;
}
