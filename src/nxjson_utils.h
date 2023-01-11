#ifndef NXJSON_UTILS_H_
#define NXJSON_UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#include "error.h"
#include "memory.h"
#include "nxjson.h"
#include "slurp_file.h"

#define nx_json_for_each(VAR, OBJECT) \
  for (const nx_json* VAR = OBJECT->val.children.first; VAR; VAR = VAR->next)

static inline Error* nx_json_get_str(const char** out, const nx_json* node) {
  if (node->type == NX_JSON_STRING) {
    *out = node->val.text;
    return err_success();
  }
  return err_string(0, "Not a string");
}

static inline Error* nx_json_get_array(const nx_json* node) {
  if (node->type != NX_JSON_ARRAY)
    return err_string(0, "Not an array");
  return err_success();
}

static inline Error* nx_json_get_object(const nx_json* node) {
  if (node->type != NX_JSON_OBJECT)
    return err_string(0, "Not an object");
  return err_success();
}

static inline Error* nx_json_parse_file(const nx_json** out, const char* file) {
  char buf[16384];
  int len = slurp_file(buf, sizeof(buf) - 1, file);
  if (len < 0)
    return err_stdlib(0, file);

  char* tmp = Temp_Strdup(buf);

  *out = nx_json_parse_utf8(tmp);
  if (! *out)
    return err_nxjson(0, NULL);

  return err_success();
}

#endif
