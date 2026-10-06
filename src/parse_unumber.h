#ifndef NBFC_PARSE_UNUMBER_H_
#define NBFC_PARSE_UNUMBER_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/*
 * Parses an unsigned integer value from a string with strict and predictable
 * semantics.
 *
 * The function accepts only:
 *  - Positive decimal numbers (base 10)
 *  - Positive hexadecimal numbers (base 16) with an explicit "0x" or "0X" prefix.
 *  - Positive binary numbers (base 2) with an explicit "0b" or "0B" prefix.
 *
 * Hexadecimal/binary parsing is performed explicitly instead of relying on
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
  // 128 bytes are more than enough for any valid integer. If the source string
  // is longer, truncating it is harmless because strtoll() will report ERANGE.
  char buf[128];
  size_t i = 0;
  uint64_t val;
  int base = 10;
  int is_negative = 0;
  char* end = "";
  errno = 0;

  // We accept exactly one sign
  if (*s == '+') {
    buf[i++] = *s;
    ++s;
  }
  else if (*s == '-') {
    is_negative = 1;
    ++s;
  }

  // Leading zero
  if (*s == '0') {
    ++s;

    // No more chars
    if (*s == '\0') {
      val = 0;
      goto check;
    }

    // Binary number
    if (*s == 'b' || *s == 'B') {
      ++s;
      base = 2;
      goto parse;
    }

    // Hexadecimal number
    if (*s == 'x' || *s == 'X') {
      ++s;
      base = 16;
      goto parse;
    }

    // Octal number
    if (*s >= '0' && *s <= '9') {
      *errmsg = "octal values not supported";
      return 0;
    }
  }

parse:
  // We (maybe) have a prefix, but the remaining string is empty
  if (*s == '\0') {
    errno = EINVAL;
    goto check;
  }

  // Only accept hexadecimal chars; no whitespace
  for (const char* c = s; *c && i < sizeof(buf) - 1; ++c)
    if ((*c >= '0' && *c <= '9') ||
        (*c >= 'A' && *c <= 'F') ||
        (*c >= 'a' && *c <= 'f'))
    {
      buf[i++] = *c;
    }
    else
    {
      *errmsg = strerror(EINVAL);
      return 0;
    }

  buf[i] = '\0';
  val = strtoull(buf, &end, base);

check:
  if (errno)
    *errmsg = strerror(errno);
  else if (*end)
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
