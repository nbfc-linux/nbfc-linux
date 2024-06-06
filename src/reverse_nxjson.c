#include "reverse_nxjson.h"

#include <stdio.h>
#include <string.h>

#define ADD_KEY_NOT_NULL()                                                     \
  if (nx->key != NULL) {                                                       \
    StringBuf_AddCh(s, '"');                                                 \
    StringBuf_Printf(s, "%s", nx->key);                                       \
    StringBuf_Printf(s, "%s", "\": ");                                               \
  }

#define ADD_INDENTION(N) \
    StringBuf_Printf(s, "\n%*s", N, " ")

char *nx_json_to_string(const nx_json *nx, StringBuf* s, int indent) {
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
        StringBuf_Printf(s, "%s", nx->val.text);
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
