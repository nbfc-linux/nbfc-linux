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
 * The function accepts only:
 *  - Decimal numbers (base 10)
 *  - Hexadecimal numbers (base 16) with an explicit "0x" or "0X" prefix.
 *  - Binary numbers (base 2) with an explicit "0b" or "0B" prefix.
 *
 * Hexadecimal/binary parsing is performed explicitly instead of relying on
 * base-0 conversion. This is intentional to avoid accepting octal values
 * such as "0777".
 *
 * Octal numbers are rarely used and can easily lead to subtle bugs or security
 * issues when users are unaware that leading zeros imply octal interpretation.
 * This also protects against cases where a user forgets the 'x' in "0x".
 */
static int64_t parse_number(const char* s, int64_t min, int64_t max, const char** errmsg) {
  // 128 bytes are more than enough for any valid integer. If the source string
  // is longer, truncating it is harmless because strtoll() will report ERANGE.
  char buf[128];
  size_t i = 0;
  int64_t val;
  int base = 10;
  char* end = "";
  errno = 0;

  // We accept exactly one sign
  if (*s == '+' || *s == '-') {
    buf[i++] = *s;
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
  val = strtoll(buf, &end, base);

check:
  if (errno)
    *errmsg = strerror(errno);
  else if (*end)
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
