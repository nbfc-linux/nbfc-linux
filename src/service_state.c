#include "service_state.h"

#include "nbfc.h"
#include "macros.h"
#include "memory.h"
#include "buffer.h"
#include "trace.h"
#include "nxjson_utils.h"
#include "nxjson_write.h"

#include <sys/stat.h>

ServiceState service_state = {0};

Error ServiceState_Init() {
  Error e;
  Trace* trace = (Trace*) Buffer_Get(sizeof(Trace));
  char* file_content = Buffer_Get(NBFC_MAX_FILE_SIZE);
  const nx_json* js = NULL;

  Trace_Init(trace);
  Trace_Push(trace, NBFC_STATE_FILE);

  e = nx_json_parse_file(&js, file_content, NBFC_MAX_FILE_SIZE, NBFC_STATE_FILE);
  if (e)
    goto err;

  e = ServiceState_FromJson(&service_state, js);
  if (e)
    goto err;

  e = ServiceState_ValidateFields(&service_state);
  if (e)
    goto err;

  for_each_array(float*, f, service_state.TargetFanSpeeds) {
    Trace_Push(trace, "TargetFanSpeeds[%d]", PTR_DIFF(f, service_state.TargetFanSpeeds.data));

    if (*f > 100.0f) {
      Log_Warn("%s: Value cannot be greater than 100.0", trace->buf);
      *f = 100.0f;
    }

    if (*f < 0.0f && *f != -1.0f) {
      Log_Warn("%s: Please use `-1' for selecting auto mode", trace->buf);
      *f = -1.0f;
    }

    Trace_Pop(trace);
  }

err:
  nx_json_free(js);

  if (e)
    e = err_chain_string(e, trace->buf);

  Buffer_Release(file_content, NBFC_MAX_FILE_SIZE);
  Buffer_Release((char*) trace, sizeof(Trace));

  return e;
}

Error ServiceState_Write() {
  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);

  if (service_state.TargetFanSpeeds.size) {
    nx_json* fanspeeds = create_json_array("TargetFanSpeeds", o);

    for_each_array(float*, f, service_state.TargetFanSpeeds)
      create_json_double(NULL, fanspeeds, *f);
  }

  int fd = open(NBFC_STATE_FILE, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (fd == -1)
    return err_stdlib(NBFC_STATE_FILE);

  NX_JSON_Write write_obj = NX_JSON_Write_Init(fd, WriteMode_Write);
  nx_json_write(&write_obj, o, 0);
  nx_json_free(o);
  close(fd);

  if (write_obj.success)
    return err_success();
  else
    return err_stdlib(NBFC_STATE_FILE);
}

void ServiceState_Free() {
  Mem_Free(service_state.TargetFanSpeeds.data);
  memset(&service_state, 0, sizeof(service_state));
}
