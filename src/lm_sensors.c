#include "lm_sensors.h"

#include "nbfc.h"
#include "error.h"

#include <stdio.h>
#include <string.h>
#include <sensors/error.h>
#include <sensors/sensors.h>

static const sensors_chip_name* LM_Sensors_ChipName;
static Error* LM_Sensors_Error;

static Error* err_sensors(Error* e, const int code, const char* message) {
  e = err_allocate(e);
  e->system = ErrorSystem_Sensors;
  e->value.code = code;
  if (message)
    return err_string(e, message);
  return e;
}

static void LM_Sensors_parse_error(const char* err, int lineno) {
  Error* e = err_string(0, "Error parsing lm_sensors configuration file");
  e = err_integer(e, lineno);
  e = err_string(e, err);
  LM_Sensors_Error = e;
}

static void LM_Sensors_parse_error_wfn(const char* err, const char* filename, int lineno) {
  Error* e = err_string(0, "Error parsing lm_sensors configuration file");
  e = err_string(e, filename);
  e = err_integer(e, lineno);
  e = err_string(e, err);
  LM_Sensors_Error = e;
}

static void LM_Sensors_fatal_error(const char* proc, const char* err) {
  fprintf(stderr, "Fatal error in `%s': %s\n", proc, err);
  exit(NBFC_EXIT_FATAL);
}

Error* LM_Sensors_Init() {
  sensors_fatal_error     = LM_Sensors_fatal_error;
  sensors_parse_error     = LM_Sensors_parse_error;
  sensors_parse_error_wfn = LM_Sensors_parse_error_wfn;

  LM_Sensors_Error = NULL;
  const int err = sensors_init(NULL);
  if (LM_Sensors_Error)
    return LM_Sensors_Error;
  if (err)
    return err_sensors(NULL, err, NULL);

  for (int n=0; (LM_Sensors_ChipName = sensors_get_detected_chips(NULL, &n));)
    if (! strcmp(LM_Sensors_ChipName->prefix, "coretemp"))
      return err_success();

  LM_Sensors_ChipName = NULL;
  return err_string(0, "No chip with name 'coretemp' found");
}

void LM_Sensors_Cleanup() {
  sensors_cleanup();
}

Error* LM_Sensors_GetTemperature(float* out) {
  const  sensors_feature* feature;
  const  sensors_subfeature* sub_feature;
  double val, total = 0;
  int    count = 0;

  for (int n = 0; (feature = sensors_get_features(LM_Sensors_ChipName, &n));)
    if (feature->type == SENSORS_FEATURE_TEMP)
      if ((sub_feature = sensors_get_subfeature(LM_Sensors_ChipName, feature, SENSORS_SUBFEATURE_TEMP_INPUT)))
        if (sensors_get_value(LM_Sensors_ChipName, sub_feature->number, &val) == 0) {
          total += val;
          count++;
        }

  if (! count)
    return err_string(0, "Number of temperatures == 0");

  *out = total / count;
  return err_success();
}

Sensor_VTable LM_Sensors_VTable = {
  LM_Sensors_Init,
  LM_Sensors_Cleanup,
  LM_Sensors_GetTemperature
};

