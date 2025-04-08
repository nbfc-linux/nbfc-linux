#ifndef CONFIG_H_
#define CONFIG_H_

/* Default config.h for builds without ./configure */

/* Define to 1 to enable the '/sys/kernel/debug/ec/ec0/io' embedded controller */
#define ENABLE_EC_SYS 1

/* Define to 1 to enable the '/dev/ec' embedded controller */
#define ENABLE_EC_ACPI 1

/* Define to 1 to enable the '/dev/port' embedded controller */
#define ENABLE_EC_DEV_PORT 1

/* Define to 1 to enable the 'dummy' embedded controller */
#define ENABLE_EC_DUMMY 1

/* Define to 1 to enable debugging of the embedded controller */
#define ENABLE_EC_DEBUG 1

#endif
