#include "acpi_call.h"

#include <errno.h>  // errno, EINVAL, ENOBUFS, ENODATA
#include <stdio.h>  // snprintf
#include <string.h> // strlen

#include "macros.h"
#include "process.h"
#include "file_utils.h"

#define ACPI_CALL_FILE         "/proc/acpi/call"
#define ACPI_CALL_MODPROBE_CMD "modprobe acpi_call"

Error AcpiCall_Open() {
  return Process_Call(ACPI_CALL_MODPROBE_CMD);
}

Error AcpiCall_Call(const char* cmd, ssize_t cmd_len, uint64_t* out) {
  ssize_t ret;
  char output[4096];
  char* end;

  ret = write_file(ACPI_CALL_FILE, O_WRONLY, 0, cmd, cmd_len);

  if (ret == -1)
    return err_stdlib(ACPI_CALL_FILE);

  ret = slurp_file(output, sizeof(output), ACPI_CALL_FILE);

  if (ret == -1)
    return err_stdlib(ACPI_CALL_FILE);

  if (! output[0]) {
    errno = ENODATA;
    return err_stdlib(ACPI_CALL_FILE);
  }

  if (output[0] == 'E') {
    const char* errmsg = output + STRLEN("Error: ");
    return err_stringf("%s: %s", ACPI_CALL_FILE, errmsg);
  }

  errno = 0;
  *out = strtoull(output, &end, 0);

  if (errno)
    return err_stdlib(ACPI_CALL_FILE);

  if (*end) {
    errno = EINVAL;
    return err_stdlib(ACPI_CALL_FILE);
  }

  return err_success();
}

Error AcpiCall_CallTemplate(const char* template_, uint64_t value, uint64_t* out) {
  char cmd[8192];
  ssize_t cmd_len = 0;

  // If every char in template is a placeholder ("$"), will it still fit in `cmd`?
  if (strlen(template_) * STRLEN("0x1122334455667788") >= sizeof(cmd)) {
    errno = ENOBUFS;
    return err_stdlib(ACPI_CALL_FILE);
  }

  for (; *template_; ++template_) {
    if (*template_ == '$')
      cmd_len += snprintf(cmd + cmd_len, 99, "0x%lX", value);
    else 
      cmd[cmd_len++] = *template_;
  }

  return AcpiCall_Call(cmd, cmd_len, out);
}
