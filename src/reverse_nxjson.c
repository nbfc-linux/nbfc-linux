#include "nxjson.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IF_KEY_NOT_NULL                                                        \
  if (nx->key != NULL) {                                                       \
    strcat(s, "\"");                                                           \
    strcat(s, nx->key);                                                        \
    strcat(s, "\": ");                                                         \
  }

char *nx_json_to_string(const nx_json *nx) {
  const int SIZE = (int)1e13;
  char *s = malloc(SIZE);
  strcpy(s, "");
  while (nx != NULL) {
    if (nx->type == NX_JSON_OBJECT) {
      IF_KEY_NOT_NULL
      strcat(s, "{");
      strcat(s, nx_json_to_string(nx->val.children.first));
      strcat(s, "}");
    } else if (nx->type == NX_JSON_ARRAY) {
      IF_KEY_NOT_NULL
      strcat(s, "[");
      strcat(s, nx_json_to_string(nx->val.children.first));
      strcat(s, "]");
    } else {
      if (nx->type == NX_JSON_STRING) {
        IF_KEY_NOT_NULL
        strcat(s, "\"");
        strcat(s, nx->val.text);
        strcat(s, "\"");
      } else if (nx->type == NX_JSON_BOOL) {
        IF_KEY_NOT_NULL
        strcat(s, nx->val.u ? "true" : "false");
      } else if (nx->type == NX_JSON_INTEGER) {
        IF_KEY_NOT_NULL
        char i[snprintf(NULL, 0, "%ld", nx->val.i) + 1];
        sprintf(i, "%ld", nx->val.i);
        strcat(s, i);
      } else if (nx->type == NX_JSON_DOUBLE) {
        IF_KEY_NOT_NULL
        char dbl[snprintf(NULL, 0, "%lf", nx->val.dbl) + 1];
        sprintf(dbl, "%lf", nx->val.dbl);
        strcat(s, dbl);
      } else {
        IF_KEY_NOT_NULL
        strcat(s, "null");
      }
    }
    nx = nx->next;
    if (nx != NULL)
      strcat(s, ", ");
  }
  return s;
}
