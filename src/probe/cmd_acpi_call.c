#include "../acpi_call.h"
#include "../client/check_root.h"

#include <stdio.h>  // printf, snprintf
#include <string.h> // strlen, memcpy
#include <stdbool.h>

#define ACPI_CALL_MAX_CMD_LEN 4096

static const struct cli99_Option AcpiCall_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-m|--map", Option_Map,              cli99_RequiredArgument},
  {"-d|--dry", Option_Dry,              cli99_NoArgument      },
  {"method",   Option_AcpiCallMethod,   cli99_NormalPositional},
  {"arg1",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg2",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg3",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg4",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg5",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg6",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg7",     Option_AcpiCallArgument, cli99_NormalPositional},
  {"arg8",     Option_AcpiCallArgument, cli99_NormalPositional},
  cli99_Options_End()
};

/*
 * Example:
 *   ("\\_SB.PC00.LPCB.EC0.FANG", "FANG") -> true
 *   ("\\_SB.PC00.LPCB.EC0.FANG", "ANG")  -> false
 */
static bool AcpiCall_MethodPartialCompare(const char* full, const char* partial) {
  ssize_t full_idx = (ssize_t) strlen(full);
  ssize_t part_idx = (ssize_t) strlen(partial);

  while (--full_idx >= 0 && --part_idx >= 0) {
    if (full[full_idx] != partial[part_idx])
      return false;
  }

  if (part_idx >= 0)
    return false;

  if (full_idx == -1)
    return true;

  return (full[full_idx] == '.' || full[full_idx] == '\\');
}

static Error AcpiCall_ResolveMethod(const char* name, const char** resolved) {
  if (name[0] == '\\') {
    *resolved = name;
    return err_success();
  }

  *resolved = NULL;
  for_each_array(str*, method, Map.methods) {
    if (AcpiCall_MethodPartialCompare(*method, name)) {
      if (*resolved)
        return err_stringf("%s: Ambiguous method name", name);

      *resolved = *method;
    }
  }

  if (! *resolved)
    *resolved = name;

  return err_success();
}

/*
 * Tries to resolve and replace the method name in `in_out`.
 */
static Error AcpiCall_ResolveAndReplaceMethod(char* in_out, int* new_size) {
  Error e;
  char buf_cmd[ACPI_CALL_MAX_CMD_LEN] = {0};
  char buf_args[ACPI_CALL_MAX_CMD_LEN] = {0};
  const size_t method_name_len = strcspn(in_out, " \t\n");
  const char* resolved;

  // Already zero-initialized; no need for appending '\0'
  memcpy(buf_cmd, in_out, method_name_len);

  // Resolve method
  e = AcpiCall_ResolveMethod(buf_cmd, &resolved);
  if (e)
    return e;

  // We have no arguments
  if (in_out[method_name_len] == '\0') {
    *new_size = snprintf(in_out, ACPI_CALL_MAX_CMD_LEN, "%s", resolved);
  }
  // We have arguments
  else {
    const char* const args_start = in_out + method_name_len + 1;
    memcpy(buf_args, args_start, strlen(args_start));
    *new_size = snprintf(in_out, ACPI_CALL_MAX_CMD_LEN, "%s %s", resolved, buf_args);
  }

  if (*new_size == -1 || *new_size >= ACPI_CALL_MAX_CMD_LEN) {
    errno = ENOBUFS;
    return err_stdlib(NULL);
  }

  return err_success();
}

static int AcpiCall(void) {
  check_root();

  Error e;
  char cmd[ACPI_CALL_MAX_CMD_LEN];
  char fmt[] = "%s 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX 0x%lX";
  fmt[2 + options.acpi_call_args_size * 6] = '\0';

  int cmd_len = snprintf(cmd, sizeof(cmd), fmt,
    options.acpi_call_method,
    options.acpi_call_args[0],
    options.acpi_call_args[1],
    options.acpi_call_args[2],
    options.acpi_call_args[3],
    options.acpi_call_args[4],
    options.acpi_call_args[5],
    options.acpi_call_args[6],
    options.acpi_call_args[7]
  );

  if (cmd_len == -1 || cmd_len >= (int) sizeof(cmd)) {
    Log_Error("Method (including arguments) is too long");
    return NBFC_EXIT_FAILURE;
  }

  if (options.map) {
    e = Map_Load(options.map);
    if (e) {
      e = err_chain_string(e, options.map);
      e_die();
    }

    e = AcpiCall_ResolveAndReplaceMethod(cmd, &cmd_len);
    e_die();
  }

  if (options.dry) {
    printf("%s\n", cmd);
    return NBFC_EXIT_SUCCESS;
  }

  e = AcpiCall_Open();
  e_die();

  char* out;
  e = AcpiCall_CallRaw(cmd, cmd_len, &out);
  e_die();
  printf("%s\n", out);

  return NBFC_EXIT_SUCCESS;
}
