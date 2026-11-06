#include "../nbfc.h"
#include "../memory.h"
#include "../nxjson.h"

#include "check_root.h"
#include "client_global.h"
#include "service_control.h"

static Error SuspendResume_SendCommand(const char* command) {
  Error e;
  char* buf = NULL;
  const nx_json* out = NULL;

  nx_json cmd  = {0};
  cmd.type     = NX_JSON_STRING;
  cmd.key      = "Command";
  cmd.val.text = command;

  nx_json obj             = {0};
  obj.type                = NX_JSON_OBJECT;
  obj.val.children.length = 1;
  obj.val.children.first  = &cmd;
  obj.val.children.last   = &cmd;

  e = Client_Communicate(&obj, &buf, &out);
  if (e)
    goto error;

  if (out->type != NX_JSON_OBJECT) {
    e = err_string("Not a JSON object");
    goto error;
  }

  const nx_json* err = nx_json_get(out, "Error");
  if (err) {
    if (err->type != NX_JSON_STRING) {
      e = err_string("\"Error\" is not a string");
      goto error;
    }

    e = err_stringf("Service returned: %s", err->val.text);
    goto error;
  }

error:

#if STRICT_CLEANUP
  Mem_Free(buf);
  nx_json_free(out);
#endif

  return e;
}

static int Suspend(void) {
  Error e;

  check_root();

  e = SuspendResume_SendCommand("suspend");
  if (e) {
    Log_Error("%s", err_print_all(e));
    Log_Error("Cancelled suspend");
    return NBFC_EXIT_FAILURE;
  }

  Log_Info("Service suspended. No fan control will be performed until `nbfc resume` is called.");

  return NBFC_EXIT_SUCCESS;
}

static int Resume(void) {
  Error e;

  check_root();

  e = SuspendResume_SendCommand("resume");
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  return NBFC_EXIT_SUCCESS;
}
