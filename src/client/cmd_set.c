#include <string.h> // strcmp

#include "service_control.h"
#include "client_global.h"

#include "../help/client.help.h"
#include "../nxjson_utils.h"
#include "../memory.h"
#include "../nbfc.h"

const cli99_option set_options[] = {
  cli99_include_options(&main_options),
  {"-a|--auto",  Option_Set_Auto,  0},
  {"-s|--speed", Option_Set_Speed, 1},
  {"-f|--fan",   Option_Set_Fan,   1},
  cli99_options_end()
};

struct {
  int   fan;
  float speed;
} Set_Options = {
  -1,
  -2.0
};

int Set() {
  if (Set_Options.speed == -2.0) {
    printf(CLIENT_SET_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }

  if (Service_Get_PID() == -1) {
    Log_Error("Service not running\n");
    return NBFC_EXIT_FAILURE;
  }

  nx_json root = {0};
  nx_json* in = create_json_object(NULL, &root);
  create_json_string("Command", in, "set-fan-speed");

  if (Set_Options.fan != -1)
    create_json_integer("Fan", in, Set_Options.fan);

  if (Set_Options.speed == -1.0)
    create_json_string("Speed", in, "auto");
  else
    create_json_double("Speed", in, Set_Options.speed);

  char* buf = NULL;
  const nx_json* out = NULL;
  Error* e = Client_Communicate(in, &buf, &out);
  if (e)
    goto error;

  if (out->type != NX_JSON_OBJECT) {
    e = err_string(0, "Not a JSON object");
    goto error;
  }

  const nx_json* err = nx_json_get(out, "Error");
  if (err) {
    if (err->type != NX_JSON_STRING) {
      e = err_string(0, "'Error' is not a string");
      goto error;
    }

    Log_Error("Service returned: %s\n", err->val.text);
    return NBFC_EXIT_FAILURE;
  }

  const nx_json* status = nx_json_get(out, "Status");
  if (! status) {
    e = err_string(0, "Missing status in JSON output");
    goto error;
  }

  if (status->type != NX_JSON_STRING) {
    e = err_string(0, "Status: not a JSON string");
    goto error;
  }

  if (strcmp(status->val.text, "OK")) {
    e = err_string(0, "Status != OK");
    goto error;
  }

error:
  nx_json_free(in);
  nx_json_free(out);
  Mem_Free(buf);

  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  return NBFC_EXIT_SUCCESS;
}
