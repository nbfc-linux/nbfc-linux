#include "reverse_nxjson.h"

#include "nbfc.h"

#include <stdio.h>
#include <string.h>

#define ADD_KEY_NOT_NULL()                                                     \
  if (nx->key != NULL) {                                                       \
    StringBuf_AddCh(s, '"');                                                   \
    StringBuf_Printf(s, "%s", nx->key);                                        \
    StringBuf_Printf(s, "%s", "\": ");                                         \
  }

#define ADD_INDENTION(N) \
    StringBuf_Printf(s, "\n%*s", N, "")

static const char* Json_EscapeString(char*, const size_t, const char*);

char *nx_json_to_string(const nx_json *nx, StringBuf* s, int indent) {
  static char buf[NBFC_MAX_FILE_SIZE];

  while (nx != NULL) {
    if (nx->type == NX_JSON_OBJECT) {
      ADD_INDENTION(indent);
      ADD_KEY_NOT_NULL();
      StringBuf_AddCh(s, '{');
      nx_json_to_string(nx->val.children.first, s, indent + 3);
      ADD_INDENTION(indent);
      StringBuf_AddCh(s, '}');
    } else if (nx->type == NX_JSON_ARRAY) {
      ADD_INDENTION(indent);
      ADD_KEY_NOT_NULL();
      StringBuf_AddCh(s, '[');
      nx_json_to_string(nx->val.children.first, s, indent + 3);
      ADD_INDENTION(indent);
      StringBuf_AddCh(s, ']');
    } else {
      if (nx->type == NX_JSON_STRING) {
        ADD_INDENTION(indent);
        ADD_KEY_NOT_NULL();
        StringBuf_AddCh(s, '"');
        Json_EscapeString(buf, sizeof(buf), nx->val.text);
        StringBuf_Printf(s, "%s", buf);
        StringBuf_AddCh(s, '"');
      } else if (nx->type == NX_JSON_BOOL) {
        ADD_INDENTION(indent);
        ADD_KEY_NOT_NULL();
        StringBuf_Printf(s, "%s", nx->val.u ? "true" : "false");
      } else if (nx->type == NX_JSON_INTEGER) {
        ADD_INDENTION(indent);
        ADD_KEY_NOT_NULL();
        StringBuf_Printf(s, "%ld", nx->val.i);
      } else if (nx->type == NX_JSON_DOUBLE) {
        ADD_INDENTION(indent);
        ADD_KEY_NOT_NULL();
        StringBuf_Printf(s, "%lf", nx->val.dbl);
      } else if (nx->type == NX_JSON_NULL) {
        ADD_INDENTION(indent);
        ADD_KEY_NOT_NULL();
        StringBuf_Printf(s, "%s", "null");
      }
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
