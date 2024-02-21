#ifndef NBFC_SERVICE_H_
#define NBFC_SERVICE_H_

#include "error.h"
#include "model_config.h"

#include <stdbool.h>

typedef struct Service_Options Service_Options;
struct Service_Options {
  EmbeddedControllerType embedded_controller_type;
  bool                   fork;
  bool                   read_only;
  int                    debug;
  const char*            service_config;
  const char*            state_file;
};

extern Service_Options options;

Error* Service_Init();
void   Service_Loop();
void   Service_Cleanup();

#endif
