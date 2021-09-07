#ifndef NBFC_NBFC_H_
#define NBFC_NBFC_H_

#include "model_config.h"
#include "macros.h"
#include "error.h"
#include "ec.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define NBFC_VERSION                     "0.1.6"
#define NBFC_MAX_FILE_SIZE               32768
#define NBFC_TEMPERATURE_FILTER_TIMESPAN 6000  /*ms*/
#define NBFC_CONFIGS_DIR                 DATADIR "/nbfc/configs"
#define NBFC_SERVICE_CONFIG              CONFDIR "/nbfc/nbfc.json"
#define NBFC_PID_FILE                    "/var/run/nbfc_service.pid"
#define NBFC_STATE_FILE                  "/var/run/nbfc_service.state.json"

#define NBFC_EXIT_SUCCESS EXIT_SUCCESS
#define NBFC_EXIT_FAILURE EXIT_FAILURE
#define NBFC_EXIT_CMDLINE 2
#define NBFC_EXIT_INIT    3
#define NBFC_EXIT_FATAL   5

typedef struct Sensor_VTable Sensor_VTable;
struct Sensor_VTable {
  Error*   (*Init)();
  void     (*Cleanup)();
  Error*   (*GetTemperature)(float*);
};

extern EC_VTable* ec;

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/// Read `file` into `buf`
static inline ssize_t slurp_file(char* buf, size_t size, const char* file) {
  ssize_t   nread = -1;
  const int fd = open(file, O_RDONLY);
  if (fd >= 0) {
    if ((nread = read(fd, buf, size)) >= 0)
      buf[nread] = '\0';
    else if (nread == (ssize_t) size)
      (errno = ENOBUFS), nread = -1;
    close(fd);
  }
  return nread;
}

#endif
