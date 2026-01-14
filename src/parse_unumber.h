#ifndef PARSE_UNUMBER_H_
#define PARSE_UNUMBER_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*
 * Parses an unsigned integer value from a string with strict and predictable
 * semantics.
 *
 * The function accepty only:
 *  - Positive decimal numbers (base 10)
 *  - Positive hexadecimal numbers (base 16) with an explicit "0x" or "0X" prefix.
 *
 * Hexadecimal parsing is performed explicitly instead of relying on
 * base-0 conversion. This is intentional to avoid accepting octal values
 * such as "0777".
 *
 * Octal numbers are rarely used and can easily lead to subtle bugs or security
 * issues when users are unaware that leading zeros imply octal interpretation.
 * This also protects against cases where a user forgets the 'x' in "0x".
 *
 * Unlike strtoull(), negative values are explicitly rejected and treated
 * as an error; they are not converted or wrapped into unsigned values.
 */
static uint64_t parse_unumber(const char* s, uint64_t min, uint64_t max, const char** errmsg) {
  int base = 10;
  int start = 0;
  int is_negative = 0;

  if (s[0] == '-')
    is_negative = 1;

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
  uint64_t val = strtoull(s, &end, base);

  if (errno)
    *errmsg = strerror(errno);
  else if (!*s || *end)
    *errmsg = strerror(EINVAL);
  else if (is_negative && val)
    *errmsg = "value is negative";
  else if (val < min)
    *errmsg = "value too small";
  else if (val > max)
    *errmsg = "value too large";
  else
    *errmsg = NULL;

  return val;
}

#endif
