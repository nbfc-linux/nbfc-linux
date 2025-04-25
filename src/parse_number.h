#ifndef PARSE_NUMBER_H_
#define PARSE_NUMBER_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static int64_t parse_number(const char* s, int64_t min, int64_t max, const char** errmsg) {
  errno = 0;
  char* end;
  int64_t val = strtoll(s, &end, 0);

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
