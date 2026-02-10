#ifndef NBFC_NXJSON_WRITE_H_
#define NBFC_NXJSON_WRITE_H_

#include "nxjson.h"

#include <stdbool.h>

bool nxjson_send_to_fd(const nx_json*, int);
bool nxjson_write_to_fd(const nx_json*, int);

#endif
