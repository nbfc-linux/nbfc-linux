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
  const nx_json* js = NULL;
  Error* e = nx_json_parse_file(&js, file);
  if (e)
    return err_string(e, file);

  e = ServiceConfig_FromJson(&service_config, js);
  if (e)
    return err_string(e, file);

  e = ServiceConfig_ValidateFields(&service_config);
  if (e)
    return err_string(e, file);

  for_each_array(float*, f, service_config.TargetFanSpeeds) {
    if (*f > 100.00f) {
      e = err_string(0, "TargetFanSpeed > 100");
      return e;
    }
  }

  return err_success();
}
