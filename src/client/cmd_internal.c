#include "../nbfc.h"
#include "../sleep.h"
#include "../memory.h"
#include "../str_functions.h"

#include "service_control.h"
#include "client_global.h"

#include <stdio.h> // printf


const struct cli99_Option Internal_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"internal_command", Option_Internal_Command,   cli99_NormalPositional},
  cli99_Options_End()
};

const struct cli99_Option Internal_System_Sleep_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"--sleep",  Option_Internal_System_Sleep_Sleep,  cli99_NoArgument },
  {"--resume", Option_Internal_System_Sleep_Resume, cli99_NoArgument },
  cli99_Options_End()
};

enum Internal_Command {
  Internal_Command_None,
  Internal_Command_System_Sleep,
};
typedef enum Internal_Command Internal_Command;

enum Internal_System_Sleep_Command {
  Internal_Command_System_Sleep_None,
  Internal_Command_System_Sleep_Sleep,
  Internal_Command_System_Sleep_Resume,
};
typedef enum Internal_System_Sleep_Command Internal_System_Sleep_Command;


struct {
  Internal_Command command;
  Internal_System_Sleep_Command sleep_command;
} Internal_Options = {
  Internal_Command_None,
  Internal_Command_System_Sleep_None,
};

Internal_Command Internal_Command_FromString(const char* s) {
  if (! strcmp(s, "system-sleep"))  return Internal_Command_System_Sleep;
  return Internal_Command_None;
}


static int Internal_System_Sleep(void) {
  if (Internal_Options.sleep_command == Internal_Command_System_Sleep_None) {
    Log_Error("Missing option: %s|%s", "--sleep", "--resume");
    return NBFC_EXIT_CMDLINE;
  }

  nx_json root = {0};
  nx_json* in = create_json_object(NULL, &root);
  create_json_string("Command", in, "set-system-sleeping");

  switch (Internal_Options.sleep_command) {
    case Internal_Command_System_Sleep_Sleep:
      create_json_bool("Sleeping", in, true);
      break;

    case Internal_Command_System_Sleep_Resume:
      create_json_bool("Sleeping", in, false);
      break;

    default:
  }


  char* buf = NULL;
  const nx_json* out = NULL;
  Error e = Client_Communicate(in, &buf, &out);
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

    Log_Error("Service returned: %s", err->val.text);
    return NBFC_EXIT_FAILURE;
  }

  const nx_json* status = nx_json_get(out, "Status");
  if (! status) {
    e = err_string("Missing status in JSON output");
    goto error;
  }

  if (status->type != NX_JSON_STRING) {
    e = err_string("Status: not a JSON string");
    goto error;
  }

  if (strcmp(status->val.text, "OK")) {
    e = err_string("Status != OK");
    goto error;
  }

error:

#if STRICT_CLEANUP
  nx_json_free(in);
  nx_json_free(out);
  Mem_Free(buf);
#endif

  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  return NBFC_EXIT_SUCCESS;
}


int Internal(void) {
  switch (Internal_Options.command) {
    case Internal_Command_System_Sleep:  return Internal_System_Sleep();
    default:
      printf("%s", CLIENT_INTERNAL_HELP_TEXT);
      return NBFC_EXIT_CMDLINE;
  }
}