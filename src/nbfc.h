#ifndef NBFC_NBFC_H_
#define NBFC_NBFC_H_

#include <stdlib.h>

#define NBFC_VERSION                     "0.1.9"
#define NBFC_MAX_FILE_SIZE               32768
#define NBFC_TEMPERATURE_FILTER_TIMESPAN 6000  /*ms*/
#define NBFC_CONFIGS_DIR                 DATADIR "/nbfc/configs"
#define NBFC_CONFIG_DIR                  CONFDIR "/nbfc"
#define NBFC_SERVICE_CONFIG              CONFDIR "/nbfc/nbfc.json"
#define NBFC_PID_FILE                    "/var/run/nbfc_service.pid"
#define NBFC_STATE_FILE                  "/var/run/nbfc_service.state.json"

#define NBFC_EXIT_SUCCESS EXIT_SUCCESS
#define NBFC_EXIT_FAILURE EXIT_FAILURE
#define NBFC_EXIT_CMDLINE 2
#define NBFC_EXIT_INIT    3
#define NBFC_EXIT_FATAL   5

#endif
