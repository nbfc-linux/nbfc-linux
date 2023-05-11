#include "service_config.h"

#include "error.h"
#include "memory.h"
#include "nxjson_utils.h"

#include <stdio.h>
#include <stdlib.h>

ServiceConfig service_config = {
  "",                                /* SelectedConfigId */
  Boolean_False,                     /* ReadOnly */
  EmbeddedControllerType_Unset,      /* EmbeddedControllerType */
  {NULL, 0}                          /* TargetFanSpeeds */
};

Error* ServiceConfig_Init(const char* file) {
  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;
  Error* e = nx_json_parse_file(&js, buf, sizeof(buf), file);
  if (e)
    return e;

  e = ServiceConfig_FromJson(&service_config, js);
  nx_json_free(js);

  if (e)
    return e;

  e = ServiceConfig_ValidateFields(&service_config);
  if (e)
    return e;

  for_each_array(float*, f, service_config.TargetFanSpeeds) {
    if (*f > 100.0f) {
      e = err_string(0, "TargetFanSpeeds: value cannot be greater than 100.0");
      e = err_string(e, file);
      e_warn();
      *f = 100.0f;
    }

    if (*f < 0.0f && *f != -1.0f) {
      e = err_string(0, "TargetFanSpeeds: Please use `-1' for selecting auto mode");
      e = err_string(e, file);
      e_warn();
      *f = -1.0f;
    }
  }

  return err_success();
}
