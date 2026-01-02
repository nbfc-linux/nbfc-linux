#include "acpi_call.h"

#include <errno.h>  // errno, EINVAL, ENOBUFS, ENODATA
#include <stdio.h>  // snprintf
#include <stdlib.h> // strtoull

#include "macros.h"
#include "process.h"
#include "file_utils.h"

#define ACPI_CALL_FILE         "/proc/acpi/call"
#define ACPI_CALL_MODPROBE_CMD "modprobe acpi_call"

Error AcpiCall_Open() {
  return Process_Call(ACPI_CALL_MODPROBE_CMD);
}

Error AcpiCall_Call(const char* cmd, uint64_t value, uint64_t* out) {
  char buf[1024];
  size_t len = 0;
  ssize_t ret;
  char* end;

  // Copy `cmd` to `buf`, replacing placeholders ($) with `value` and add
  // a newline.
  for (const char* c = cmd; *c; ++c) {
    if (*c == '$') {
      int n = snprintf(buf + len, sizeof(buf) - len, "0x%lX", value);
      if (n < 0 || (size_t)n >= sizeof(buf) - len) {
        errno = ENOBUFS;
        return err_stdlib(ACPI_CALL_FILE);
      }

      len += (size_t)n;
    }
    else {
      if (len + 1 >= sizeof(buf)) {
        errno = ENOBUFS;
        return err_stdlib(ACPI_CALL_FILE);
      }

      buf[len++] = *c;
    }
  }

  if (len + 1 >= sizeof(buf)) {
    errno = ENOBUFS;
    return err_stdlib(ACPI_CALL_FILE);
  }

  buf[len++] = '\n';

  // Write command (buf) to ACPI_CALL_FILE
  ret = write_file(ACPI_CALL_FILE, O_WRONLY, 0, buf, len);

  if (ret == -1)
    return err_stdlib(ACPI_CALL_FILE);

  // Read the contents of ACPI_CALL_FILE into buf
  ret = slurp_file(buf, sizeof(buf), ACPI_CALL_FILE);

  if (ret == -1)
    return err_stdlib(ACPI_CALL_FILE);

  // Strip whitespace from response
  while (ret && buf[ret] < 32)
    buf[ret--] = '\0';

  // Check for empty response
  if (! buf[0]) {
    errno = ENODATA;
    return err_stdlib(ACPI_CALL_FILE);
  }

  // Check for error in response
  if (buf[0] == 'E') {
    return err_stringf("%s: %s", ACPI_CALL_FILE, buf + STRLEN("Error: "));
  }

  // Convert response to uint64_t
  errno = 0;
  *out = strtoull(buf, &end, 0);

  if (errno)
    return err_stdlib(ACPI_CALL_FILE);

  if (*end) {
    errno = EINVAL;
    return err_stdlib(ACPI_CALL_FILE);
  }

  return err_success();
}
