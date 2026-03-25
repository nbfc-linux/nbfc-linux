/* Auto-generated code: ['./tools/ec_available.h.py'] */

#ifndef NBFC_EC_AVAILABLE_H_
#define NBFC_EC_AVAILABLE_H_

#include "config.h"

#if ENABLE_EC_SYS && ENABLE_EC_ACPI && ENABLE_EC_DEV_PORT && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "ec_sys, acpi_ec, dev_port, dummy"

#elif ENABLE_EC_SYS && ENABLE_EC_ACPI && ENABLE_EC_DEV_PORT
#define EC_AVAILABLE_STR "ec_sys, acpi_ec, dev_port"

#elif ENABLE_EC_SYS && ENABLE_EC_ACPI && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "ec_sys, acpi_ec, dummy"

#elif ENABLE_EC_SYS && ENABLE_EC_DEV_PORT && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "ec_sys, dev_port, dummy"

#elif ENABLE_EC_ACPI && ENABLE_EC_DEV_PORT && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "acpi_ec, dev_port, dummy"

#elif ENABLE_EC_SYS && ENABLE_EC_ACPI
#define EC_AVAILABLE_STR "ec_sys, acpi_ec"

#elif ENABLE_EC_SYS && ENABLE_EC_DEV_PORT
#define EC_AVAILABLE_STR "ec_sys, dev_port"

#elif ENABLE_EC_SYS && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "ec_sys, dummy"

#elif ENABLE_EC_ACPI && ENABLE_EC_DEV_PORT
#define EC_AVAILABLE_STR "acpi_ec, dev_port"

#elif ENABLE_EC_ACPI && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "acpi_ec, dummy"

#elif ENABLE_EC_DEV_PORT && ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "dev_port, dummy"

#elif ENABLE_EC_SYS
#define EC_AVAILABLE_STR "ec_sys"

#elif ENABLE_EC_ACPI
#define EC_AVAILABLE_STR "acpi_ec"

#elif ENABLE_EC_DEV_PORT
#define EC_AVAILABLE_STR "dev_port"

#elif ENABLE_EC_DUMMY
#define EC_AVAILABLE_STR "dummy"

#else
#define EC_AVAILABLE_STR ""
#endif

#endif
