#include "service_state.h"

#include "nbfc.h"
#include "macros.h"
#include "memory.h"
#include "trace.h"
#include "stack_memory.h"
#include "nxjson_utils.h"
#include "reverse_nxjson.h"

#include <sys/stat.h>

ServiceState service_state = {0};

Error* ServiceState_Init() {
  Error* e;
  Trace trace = {0};
  char file_content[NBFC_MAX_FILE_SIZE];
  char nxjson_memory[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;

  Trace_Push(&trace, NBFC_STATE_FILE);

  // Use memory from the stack to allocate data structures from nxjson
  StackMemory_Init(nxjson_memory, sizeof(nxjson_memory));

  e = nx_json_parse_file(&js, file_content, sizeof(file_content), NBFC_STATE_FILE);
  if (e)
    goto err;

  e = ServiceState_FromJson(&service_state, js);
  if (e)
    goto err;

  e = ServiceState_ValidateFields(&service_state);
  if (e)
    goto err;

  for_each_array(float*, f, service_state.TargetFanSpeeds) {
    Trace_Push(&trace, "TargetFanSpeeds[%d]", PTR_DIFF(f, service_state.TargetFanSpeeds.data));

    if (*f > 100.0f) {
      Log_Warn("%s: Value cannot be greater than 100.0\n", trace.buf);
      *f = 100.0f;
    }

    if (*f < 0.0f && *f != -1.0f) {
      Log_Warn("%s: Please use `-1' for selecting auto mode\n", trace.buf);
      *f = -1.0f;
    }

    Trace_Pop(&trace);
  }

err:
  nx_json_free(js);
  StackMemory_Destroy();
  if (e)
    return err_string(e, trace.buf);

  return err_success();
}

Error* ServiceState_Write() {
  nx_json root = {0};
  nx_json *o = create_json_object(NULL, &root);

  if (service_state.TargetFanSpeeds.size) {
    nx_json* fanspeeds = create_json_array("TargetFanSpeeds", o);

    for_each_array(float*, f, service_state.TargetFanSpeeds)
      create_json_double(NULL, fanspeeds, *f);
  }

  char buf[NBFC_MAX_FILE_SIZE];
  StringBuf s = { buf, 0, sizeof(buf) };
  buf[0] = '\0';

  nx_json_to_string(o, &s, 0);
  nx_json_free(o);

  if (write_file(NBFC_STATE_FILE, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH, s.s, s.size) == -1) {
    return err_stdlib(0, NBFC_STATE_FILE);
  }

  return err_success();
}

void ServiceState_Free() {
  Mem_Free(service_state.TargetFanSpeeds.data);
  memset(&service_state, 0, sizeof(service_state));
}
