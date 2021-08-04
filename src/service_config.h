#ifndef NBFC_SERVICE_CONFIG_H_
#define NBFC_SERVICE_CONFIG_H_

#include <stdbool.h>

#include "nbfc.h"
#include "nxjson.h"
#include "macros.h"
#include "error.h"

extern ServiceConfig service_config;

Error* ServiceConfig_Init(const char*);

#endif
