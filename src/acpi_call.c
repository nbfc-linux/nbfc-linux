#include "acpi_call.h"

#include <stdio.h>  // snprintf
#include <string.h> // strlen
#include <stdlib.h> // system

#include "file_utils.h"

#define ACPI_CALL_FILE          "/proc/acpi/call"
#define ACPI_CALL_MODPROBE_CMD  "modprobe acpi_call"

Error* AcpiCall_Open() {
  switch (system(ACPI_CALL_MODPROBE_CMD)) {
  case 0:  return err_success();
  case -1: return err_stdlib(0, "system()");
  default: return err_stringf(0, "Could not execute `%s'", ACPI_CALL_MODPROBE_CMD);
  }
}

Error* AcpiCall_Call(const char* cmd, ssize_t cmd_len, uint64_t* out) {
  ssize_t ret;
  char output[4096];
  char* end;

  ret = write_file(ACPI_CALL_FILE, O_WRONLY, 0, cmd, cmd_len);

  if (ret == -1)
    return err_stdlib(NULL, ACPI_CALL_FILE);

  ret = slurp_file(output, sizeof(output), ACPI_CALL_FILE);

  if (ret == -1)
    return err_stdlib(NULL, ACPI_CALL_FILE);

  if (! output[0]) {
    errno = ENODATA;
    return err_stdlib(NULL, ACPI_CALL_FILE);
  }

  if (output[0] == 'E') {
    const char* errmsg = output + (sizeof("Error: ") - 1);
    return err_stringf(NULL, "%s: %s", ACPI_CALL_FILE, errmsg);
  }

  errno = 0;
  *out = strtoull(output, &end, 0);

  if (errno)
    return err_stdlib(NULL, ACPI_CALL_FILE);

  if (*end) {
    errno = EINVAL;
    return err_stdlib(NULL, ACPI_CALL_FILE);
  }

  return err_success();
}

Error* AcpiCall_CallTemplate(const char* template, uint64_t value, uint64_t* out) {
  char cmd[8192];
  ssize_t cmd_len = 0;
  char value_str[32];
  const ssize_t value_len = snprintf(value_str, sizeof(value_str), "0x%lX", value);

  // If every char in template is a placeholder ("$"), will it still fit in `cmd`?
  if (strlen(template) * (sizeof("0x1122334455667788") - 1) > sizeof(cmd)) {
    errno = ENOBUFS;
    return err_stdlib(NULL, ACPI_CALL_FILE);
  }

  for (; *template; ++template) {
    if (*template == '$') {
      for (int j = 0; j < value_len; ++j)
        cmd[cmd_len++] = value_str[j];
    }
    else 
      cmd[cmd_len++] = *template;
  }

  return AcpiCall_Call(cmd, cmd_len, out);
}
