#ifndef NBFC_NXJSON_WRITE_H_
#define NBFC_NXJSON_WRITE_H_

#include "nxjson.h"

#include <stdbool.h>

#define NX_JSON_Write_Init(fd, mode) {fd, mode, true}

enum NX_JSON_WriteMode {
  WriteMode_Send,
  WriteMode_Write
};

struct NX_JSON_Write {
  int fd;
  enum NX_JSON_WriteMode mode;
  bool success;
};
typedef struct NX_JSON_Write NX_JSON_Write;

void nx_json_write(NX_JSON_Write*, const nx_json*, int);

#endif
