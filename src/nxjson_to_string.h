#ifndef NBFC_NXJSON_TO_STRING_H_
#define NBFC_NXJSON_TO_STRING_H_

#include "nxjson.h"
#include "stringbuf.h"

char *nx_json_to_string(const nx_json *nx, StringBuf* s, int indent);

#endif
