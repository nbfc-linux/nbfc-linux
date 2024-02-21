#ifndef NBFC_NBFC_H_
#define NBFC_NBFC_H_

#define NBFC_VERSION                     VERSION
#define NBFC_MAX_FILE_SIZE               32768
#define NBFC_TEMPERATURE_FILTER_TIMESPAN 6000 /*ms*/
#define NBFC_MODEL_CONFIGS_DIR           DATADIR "/nbfc/configs"
#define NBFC_CONFIG_DIR                  SYSCONFDIR "/nbfc"
#define NBFC_SERVICE_CONFIG              SYSCONFDIR "/nbfc/nbfc.json"
#define NBFC_PID_FILE                    RUNSTATEDIR "/nbfc_service.pid"
#define NBFC_STATE_FILE                  RUNSTATEDIR "/nbfc_service.state.json"

#define NBFC_EXIT_SUCCESS 0
#define NBFC_EXIT_FAILURE 1
#define NBFC_EXIT_CMDLINE 2
#define NBFC_EXIT_INIT    3
#define NBFC_EXIT_FATAL   5

#endif
