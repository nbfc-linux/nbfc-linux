#ifndef NBFC_PARSE_NUMBER_H_
#define NBFC_PARSE_NUMBER_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*
 * Parses a signed integer value from a string with strict and predictable
 * semantics.
 *
 * The function accepty only:
 *  - Decimal numbers (base 10)
 *  - Hexadecimal numbers (base 16) with an explicit "0x" or "0X" prefix.
 *
 * Hexadecimal parsing is performed explicitly instead of relying on
 * base-0 conversion. This is intentional to avoid accepting octal values
 * such as "0777".
 *
 * Octal numbers are rarely used and can easily lead to subtle bugs or security
 * issues when users are unaware that leading zeros imply octal interpretation.
 * This also protects against cases where a user forgets the 'x' in "0x".
 */
static int64_t parse_number(const char* s, int64_t min, int64_t max, const char** errmsg) {
  int base = 10;
  int start = 0;

  if (s[0] == '-' || s[0] == '+')
    ++start;

  if (s[start] == '0') {
    if (s[start+1] >= '0' && s[start+1] <= '9') {
      *errmsg = "octal values not supported";
      return 0;
    }

    if (s[start+1] == 'x' || s[start+1] == 'X')
      base = 16;
  }

  errno = 0;
  char* end;
  int64_t val = strtoll(s, &end, base);

  if (errno)
    *errmsg = strerror(errno);
  else if (!*s || *end)
    *errmsg = strerror(EINVAL);
  else if (val < min)
    *errmsg = "value too small";
  else if (val > max)
    *errmsg = "value too large";
  else
    *errmsg = NULL;

  return val;
}

#endif
