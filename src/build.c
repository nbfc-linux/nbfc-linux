#define _XOPEN_SOURCE 500 // unistd.h: pwrite()/pread(), string.h: strdup()
#define _DEFAULT_SOURCE   // endian.h: htole16(), le16toh()

// The data structures returned by nxjson are temporary and are loaded into proper C structs.
// We allocate memory on the stack to avoid malloc() and reduce memory usage.
#define NX_JSON_CALLOC(SIZE) ((nx_json*) StackMemory_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (StackMemory_Free((void*) (JSON)))

#include "config.h"
#include "ec.c"

#if ENABLE_EC_DEBUG
#include "ec_debug.c"
#endif

#if ENABLE_EC_DUMMY
#include "ec_dummy.c"
#endif

#if ENABLE_EC_DEV_PORT
#include "ec_linux.c"
#endif

#if ENABLE_EC_SYS || ENABLE_EC_ACPI
#include "ec_sys_linux.c"
#endif

#include "acpi_call.c"
#include "log.c"
#include "error.c"
#include "trace.c"
#include "fan.c"
#include "fan_temperature_control.c"
#include "fs_sensors.c"
#include "file_utils.c"
#include "memory.c"
#include "stack_memory.c"
#include "model_config.c"
#include "nxjson.c"
#include "nvidia.c"
#include "program_name.c"
#include "protocol.c"
#include "pidfile.c"
#include "reverse_nxjson.c"
#include "service.c"
#include "service_config.c"
#include "service_state.c"
#include "server.c"
#include "sponsor.c"
#include "temperature_filter.c"
#include "temperature_threshold_manager.c"
#include "mkdir_p.c"

#include "main.c"
