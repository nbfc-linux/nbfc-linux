#include "nxjson.h"
#include <stdio.h>
#include <string.h>
#define ADD_KEY_NOT_NULL()                                                     \
  if (nx->key != NULL) {                                                       \
    strcat(s, "\"");                                                           \
    strcat(s, nx->key);                                                        \
    strcat(s, "\": ");                                                         \
  }


char *nx_json_to_string(const nx_json *nx) {
  char s[128*1024];
  s[0] = 0;
  while (nx != NULL) {
    if (nx->type == NX_JSON_OBJECT) {
      ADD_KEY_NOT_NULL();
      strcat(s, "{");
      strcat(s, nx_json_to_string(nx->val.children.first));
      strcat(s, "}");
    } else if (nx->type == NX_JSON_ARRAY) {
      ADD_KEY_NOT_NULL();
      strcat(s, "[");
      strcat(s, nx_json_to_string(nx->val.children.first));
      strcat(s, "]");
    } else {
      if (nx->type == NX_JSON_STRING) {
        ADD_KEY_NOT_NULL();
        strcat(s, "\"");
        strcat(s, nx->val.text);
        strcat(s, "\"");
      } else if (nx->type == NX_JSON_BOOL) {
        ADD_KEY_NOT_NULL();
        strcat(s, nx->val.u ? "true" : "false");
      } else if (nx->type == NX_JSON_INTEGER) {
        ADD_KEY_NOT_NULL();
        char i[snprintf(NULL, 0, "%ld", nx->val.i) + 1];
        sprintf(i, "%ld", nx->val.i);
        strcat(s, i);
      } else if (nx->type == NX_JSON_DOUBLE) {
        ADD_KEY_NOT_NULL();
        char dbl[snprintf(NULL, 0, "%lf", nx->val.dbl) + 1];
        sprintf(dbl, "%lf", nx->val.dbl);
        strcat(s, dbl);
      } else {
        ADD_KEY_NOT_NULL();
        strcat(s, "null");
      }
    }
    nx = nx->next;
    if (nx != NULL)
      strcat(s, ", ");
  }
  return strdup(s);
}
