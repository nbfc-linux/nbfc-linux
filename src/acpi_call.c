#include "acpi_call.h"

#include <errno.h>  // errno, EINVAL, ENOBUFS, ENODATA
#include <stdio.h>  // snprintf
#include <stdlib.h> // strtoull
#include <string.h> // memset
#include <stdbool.h>

#include "macros.h"
#include "process.h"
#include "file_utils.h"

#define ACPI_CALL_FILE         "/proc/acpi/call"
#define ACPI_CALL_MODPROBE_CMD "modprobe acpi_call"
#define ACPI_CALL_PATH_MAX     16

struct AcpiCall_Path {
  uint16_t size;
  uint16_t path[ACPI_CALL_PATH_MAX];
};

static bool AcpiCall_Path_Equals(struct AcpiCall_Path* a, struct AcpiCall_Path* b) {
  const uint64_t* const a_as_integer = (const uint64_t*) a;
  const uint64_t* const b_as_integer = (const uint64_t*) b;

  // Fast compare of the first 8 bytes of AcpiCall_Path, this checks
  // - a->size == b->size
  // - a->path[0] == b->path[0]
  // - a->path[1] == b->path[1]
  // - a->path[2] == b->path[2]
  if (*a_as_integer != *b_as_integer)
    return false;

  // Because of fast compare, we start at index 3
  for (unsigned int i = 3; i < a->size; ++i)
    if (a->path[i] != b->path[i])
      return false;

  return true;
}

Error AcpiCall_Open(void) {
  if (file_exists(ACPI_CALL_FILE))
    return err_success();

  return Process_Call(ACPI_CALL_MODPROBE_CMD);
}

Error AcpiCall_CallRaw(const char* cmd, size_t cmdlen, char** out) {
  static char result[4096];
  file_op_result res;

  // Write command to ACPI_CALL_FILE
  res = write_file(ACPI_CALL_FILE, O_WRONLY, 0, cmd, cmdlen);

  if (! res.ok)
    return err_stdlib(ACPI_CALL_FILE);

  // Read the contents of ACPI_CALL_FILE into `result`
  res = slurp_file(result, sizeof(result), ACPI_CALL_FILE);

  if (! res.ok)
    return err_stdlib(ACPI_CALL_FILE);

  // Strip whitespace from response
  while (res.len && result[res.len] < 32)
    result[res.len--] = '\0';

  // Check for empty response
  if (! result[0]) {
    errno = ENODATA;
    return err_stdlib(ACPI_CALL_FILE);
  }

  // Check for error in response
  if (result[0] == 'E') {
    return err_stringf("%s: %s", ACPI_CALL_FILE, result + STRLEN("Error: "));
  }

  *out = result;
  return err_success();
}

Error AcpiCall_Call(const char* cmd, uint64_t value, uint64_t* out) {
  Error e;
  char buf[1024];
  size_t len = 0;
  char* result;
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

  e = AcpiCall_CallRaw(buf, len, &result);
  if (e)
    return e;

  // Convert response to uint64_t
  errno = 0;
  *out = strtoull(result, &end, 0);

  if (errno)
    return err_stdlib(ACPI_CALL_FILE);

  if (*end) {
    errno = EINVAL;
    return err_stdlib(ACPI_CALL_FILE);
  }

  return err_success();
}

static Error AcpiCall_Parse_Path(const char* path, struct AcpiCall_Path* out) {
  uint64_t number;
  bool having_number = false;

  memset(out, 0, sizeof(struct AcpiCall_Path));

  for (const char* p = path;;) {
    if (*p <= 32 /* whitespace or NUL */) {
      if (having_number) {
        if (out->size >= ACPI_CALL_PATH_MAX)
          return err_string("Path too long");

        if (number > UINT16_MAX)
          return err_stringf("Path: %lu: Number too large", number);

        out->path[out->size++] = (uint16_t) number;
        having_number = false;
      }

      if (!*p)
        break;

      ++p;
    }
    else if (*p >= '0' && *p <= '9') {
      number = 0;
      while (*p >= '0' && *p <= '9') {
        number = number * 10 + (*p - '0');
        ++p;
      }
      having_number = true;
    }
    else {
      return err_stringf("Path: Invalid character: '%c'", *p);
    }
  }

  if (out->size == 0)
    return err_string("Empty path");

  return err_success();
}

Error AcpiCall_GetInt(const char* result, const char* path, uint64_t* out) {
  Error e;
  struct AcpiCall_Path path_array = {0};
  struct AcpiCall_Path stack = {0};

  e = AcpiCall_Parse_Path(path, &path_array);
  if (e)
    return e;

  while (*result) {
    switch (*result) {
    case '[': /* fall through */
    case '{':
      if (stack.size >= ACPI_CALL_PATH_MAX)
        return err_string("Nesting of ACPI result too deep");

      stack.path[stack.size++] = 0;
      break;

    case ']': /* fall through */
    case '}':
      if (stack.size == 0)
        return err_string("Unbalanced brackets in ACPI result");

      stack.path[--(stack.size)] = 0;
      break;

    case ',':
      if (stack.size == 0)
        return err_string("Unexpected comma in ACPI result");

      stack.path[stack.size - 1]++;
      break;

    case '0': /* fall through */
    case '1': /* fall through */
    case '2': /* fall through */
    case '3': /* fall through */
    case '4': /* fall through */
    case '5': /* fall through */
    case '6': /* fall through */
    case '7': /* fall through */
    case '8': /* fall through */
    case '9':
      if (AcpiCall_Path_Equals(&stack, &path_array)) {
        // We don't check for errors like overflow
        *out = strtoull(result, NULL, 0);
        return err_success();
      }
    }

    ++result;
  }

  return err_stringf("No integer at path \"%s\"", path);
}
