#define _XOPEN_SOURCE   500 /* unistd.h: export pwrite()/pread() */
#define _DEFAULT_SOURCE     /* endian.h: */

#ifdef HAVE_SENSORS
#include "lm_sensors.c"
#endif

#include "fs_sensors.c"

#include "model_config.c"
#include "ec.c"
#include "error.c"
#include "fan.c"
#include "info.c"
#include "ec_debug.c"
#include "ec_dummy.c"
#include "ec_linux.c"
#include "ec_sys_linux.c"
#include "temperature_filter.c"
#include "temperature_threshold_manager.c"
#include "memory.c"
#include "service.c"
#include "service_config.c"
#include "program_name.c"

#define NX_JSON_CALLOC(SIZE) ((nx_json*) Mem_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (Mem_Free((void*) (JSON)))
#include "nxjson.c"

#include "optparse/optparse.c"

#include "main.c"
