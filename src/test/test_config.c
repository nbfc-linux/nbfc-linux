#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef DATADIR
#define DATADIR ""
#endif

#include "../config.h"
#include "../error.c"
#include "../buffer.c"
#include "../memory.c"
#include "../nxjson.c"
#include "../nxjson_utils.h"
#include "../nxjson_memory.c"
#include "../model_config.c"
#include "../file_utils.c"
#include "../log.c"
#include "../trace.c"
#include "../program_name.c"
#include "../lua_bindings.c"
#include "../acpi_call.c"

const EC_VTable* ec = NULL;

static bool test(const char* file) {
  Error e;
  Trace trace = {0};
  ModelConfig model_config;

  char file_content[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;
  const char* expected = NULL;
  const char* having = "OK";

  e = nx_json_parse_file(&js, file_content, sizeof(file_content), file);
  if (e) {
    fprintf(stderr, "Error reading file: %s\n", err_print_all(e));
    return false;
  }

  for (nx_json* node = js->val.children.first; node; node = node->next) {
    if (! strcmp(node->key, "Expected")) {
      expected = node->val.text;
      node->key = "Comment";
      break;
    }
  }

  if (! expected) {
    fprintf(stderr, "Missing \"Expected\" field");
    return false;
  }

  e = ModelConfig_FromJson(&model_config, js);
  if (e) {
    having = err_print_all(e);
  }
  else {
    e = ModelConfig_Validate(&trace, &model_config);
    if (e)
      having = err_print_all(e);
  }

  if (strcmp(having, expected)) {
    fprintf(stderr, "HAVING:   %s\n", having);
    fprintf(stderr, "EXPECTED: %s\n", expected);
    return false;
  }

  return true;
}

int main(int argc, const char* argv[]) {
  if (argc == 1) {
    fprintf(stderr, "%s: Missing argument\n", argv[0]);
    return 1;
  }

  if (argc > 2) {
    fprintf(stderr, "%s: Too much arguments\n", argv[0]);
    return 1;
  }

  return ! test(argv[1]);
}
