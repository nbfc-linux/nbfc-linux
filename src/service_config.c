#include "service_config.h"

#include "nbfc.h"
#include "error.h"
#include "memory.h"
#include "model_config.h"
#include "nxjson_utils.h"
#include "reverse_nxjson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ServiceConfig service_config = {0};

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

Error* ServiceConfig_Write() {
  nx_json root = {0};
  nx_json *o = create_json(NX_JSON_OBJECT, NULL, &root);

  if (service_config.SelectedConfigId != NULL) {
    nx_json *o1 = create_json(NX_JSON_STRING, "SelectedConfigId", o);
    o1->val.text = service_config.SelectedConfigId;
  }

  if (service_config.EmbeddedControllerType != EmbeddedControllerType_Unset) {
    nx_json *o1 = create_json(NX_JSON_STRING, "EmbeddedControllerType", o);
    o1->val.text = EmbeddedControllerType_ToString(service_config.EmbeddedControllerType);
  }

  if (service_config.TargetFanSpeeds.size) {
    nx_json *o1  = create_json(NX_JSON_ARRAY, "TargetFanSpeeds", o);

    for_each_array(float*, f, service_config.TargetFanSpeeds) {
      nx_json *o2 = create_json(NX_JSON_DOUBLE, NULL, o1);
      o2->val.dbl = *f;
    }
  }

  if (service_config.Port != int_Unset) {
    nx_json *o1 = create_json(NX_JSON_INTEGER, "Port", o);
    o1->val.i = service_config.Port;
  }

  char buf[NBFC_MAX_FILE_SIZE];
  StringBuf s = { buf, 0, sizeof(buf) };
  buf[0] = '\0';

  nx_json_to_string(o, &s, 0);

  if (write_file(NBFC_SERVICE_CONFIG, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH, s.s, s.size) == -1) {
    return err_stdlib(0, NBFC_SERVICE_CONFIG);
  }

  return err_success();
}

void ServiceConfig_Free(ServiceConfig* c) {
  Mem_Free((char*) c->SelectedConfigId);
  Mem_Free(c->TargetFanSpeeds.data);
  memset(c, 0, sizeof(*c));
}
