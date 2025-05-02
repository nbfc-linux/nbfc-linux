#ifndef NXJSON_UTILS_H_
#define NXJSON_UTILS_H_

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>

#include "error.h"
#include "nxjson.h"
#include "file_utils.h"

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

static inline Error* nx_json_parse_file(const nx_json** out, char* buf, size_t bufsize, const char* file) {
  int len = slurp_file(buf, bufsize, file);
  if (len < 0)
    return err_stdlib(0, NULL);

  *out = nx_json_parse_utf8(buf);
  if (! *out)
    return err_nxjson(0, NULL);

  return err_success();
}

static inline nx_json *create_json_string(const char* key, nx_json* parent, const char* text) {
  nx_json* json = create_json(NX_JSON_STRING, key, parent);
  json->val.text = text;
  return json;
}

static inline nx_json *create_json_integer(const char* key, nx_json* parent, nxjson_s64 integer) {
  nx_json* json = create_json(NX_JSON_INTEGER, key, parent);
  json->val.i = integer;
  return json;
}

static inline nx_json *create_json_double(const char* key, nx_json* parent, double val) {
  nx_json* json = create_json(NX_JSON_DOUBLE, key, parent);
  json->val.dbl = val;
  return json;
}

static inline nx_json *create_json_bool(const char* key, nx_json* parent, bool val) {
  nx_json* json = create_json(NX_JSON_BOOL, key, parent);
  json->val.i = val;
  return json;
}

static inline nx_json *create_json_object(const char* key, nx_json* parent) {
  nx_json* json = create_json(NX_JSON_OBJECT, key, parent);
  return json;
}

static inline nx_json *create_json_array(const char* key, nx_json* parent) {
  nx_json* json = create_json(NX_JSON_ARRAY, key, parent);
  return json;
}

#endif
