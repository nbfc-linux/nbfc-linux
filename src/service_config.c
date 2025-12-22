#include "service_config.h"

#include "nbfc.h"
#include "error.h"
#include "macros.h"
#include "memory.h"
#include "buffer.h"
#include "trace.h"
#include "model_config.h"
#include "nxjson_utils.h"
#include "nxjson_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

ServiceConfig service_config = {0};

Error ServiceConfig_Init(const char* file) {
  Error e;
  Trace trace = {0};
  char* file_content = Buffer_Get();
  const nx_json* js = NULL;

  Trace_Push(&trace, file);

  e = nx_json_parse_file(&js, file_content, BUFFER_SIZE, file);
  if (e)
    goto err;

  e = ServiceConfig_FromJson(&service_config, js);
  if (e)
    goto err;

  e = ServiceConfig_ValidateFields(&service_config);
  if (e)
    goto err;

  for_each_array(float*, f, service_config.TargetFanSpeeds) {
    Trace_Push(&trace, "TargetFanSpeeds[%d]", PTR_DIFF(f, service_config.TargetFanSpeeds.data));

    if (*f > 100.0f) {
      Log_Warn("%s: Value cannot be greater than 100.0", trace.buf);
      *f = 100.0f;
    }

    if (*f < 0.0f && *f != -1.0f) {
      Log_Warn("%s: Please use `-1' for selecting auto mode", trace.buf);
      *f = -1.0f;
    }

    Trace_Pop(&trace);
  }

  for_each_array(FanTemperatureSourceConfig*, ftsc, service_config.FanTemperatureSources) {
    Trace_Push(&trace, "FanTemperatureSources[%d]", PTR_DIFF(ftsc, service_config.FanTemperatureSources.data));

    e = FanTemperatureSourceConfig_ValidateFields(ftsc);
    if (e)
      goto err;

    for_each_array(FanTemperatureSourceConfig*, ftsc1, service_config.FanTemperatureSources) {
      if (ftsc != ftsc1 && ftsc->FanIndex == ftsc1->FanIndex) {
        e = err_string("Duplicate FanIndex");
        goto err;
      }
    }

    Trace_Pop(&trace);
  }

err:
  nx_json_free(js);
  Buffer_Release(file_content);
  if (e)
    return err_chain_string(e, trace.buf);

  return err_success();
}

Error ServiceConfig_Write(const char* file) {
  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);

  if (ServiceConfig_IsSet_SelectedConfigId(&service_config))
    create_json_string("SelectedConfigId", o, service_config.SelectedConfigId);

  if (ServiceConfig_IsSet_EmbeddedControllerType(&service_config))
    create_json_string("EmbeddedControllerType", o, EmbeddedControllerType_ToString(service_config.EmbeddedControllerType));

  if (service_config.TargetFanSpeeds.size) {
    nx_json* fanspeeds = create_json_array("TargetFanSpeeds", o);

    for_each_array(float*, f, service_config.TargetFanSpeeds)
      create_json_double(NULL, fanspeeds, *f);
  }

  if (service_config.FanTemperatureSources.size) {
    nx_json* fan_temperature_sources = create_json_array("FanTemperatureSources", o);

    for_each_array(FanTemperatureSourceConfig*, ftsc, service_config.FanTemperatureSources) {
      nx_json* fan_temperature_source = create_json_object(NULL, fan_temperature_sources);

      create_json_integer("FanIndex", fan_temperature_source, ftsc->FanIndex);

      if (FanTemperatureSourceConfig_IsSet_TemperatureAlgorithmType(ftsc))
        create_json_string("TemperatureAlgorithmType", fan_temperature_source, TemperatureAlgorithmType_ToString(ftsc->TemperatureAlgorithmType));

      if (ftsc->Sensors.size) {
        nx_json* sensors = create_json_array("Sensors", fan_temperature_source);
        for_each_array(const char**, sensor, ftsc->Sensors) {
          create_json_string(NULL, sensors, *sensor);
        }
      }
    }
  }

  int fd = open(file, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (fd == -1)
    return err_stdlib(file);

  NX_JSON_Write write_obj = NX_JSON_Write_Init(fd, WriteMode_Write);
  nx_json_write(&write_obj, o, 0);
  nx_json_free(o);
  close(fd);

  if (write_obj.success)
    return err_success();
  else
    return err_stdlib(file);
}

void ServiceConfig_Free(ServiceConfig* c) {
  Mem_Free((char*) c->SelectedConfigId);
  Mem_Free(c->TargetFanSpeeds.data);
  for_each_array(FanTemperatureSourceConfig*, ftsc, c->FanTemperatureSources) {
    for_each_array(const char**, s, ftsc->Sensors)
      Mem_Free((char*) *s);
    Mem_Free(ftsc->Sensors.data);
  }
  Mem_Free(c->FanTemperatureSources.data);

  memset(c, 0, sizeof(*c));
}
