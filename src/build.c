#define _XOPEN_SOURCE 500 /* unistd.h: export pwrite()/pread(), string.h: export strdup */
#define _DEFAULT_SOURCE   /* endian.h: */

#define NX_JSON_CALLOC(SIZE) ((nx_json*) Mem_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (Mem_Free((void*) (JSON)))

#include "ec.c"
#include "ec_debug.c"
#include "ec_dummy.c"
#include "ec_linux.c"
#include "ec_sys_linux.c"
#include "error.c"
#include "fan.c"
#include "fs_sensors.c"
#include "memory.c"
#include "model_config.c"
#include "nxjson.c"
#include "program_name.c"
#include "protocol.c"
#include "pidfile.c"
#include "service.c"
#include "service_config.c"
#include "server.c"
#include "temperature_filter.c"
#include "temperature_threshold_manager.c"

#include "main.c"
