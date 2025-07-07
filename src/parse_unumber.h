#ifndef PARSE_UNUMBER_H_
#define PARSE_UNUMBER_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static uint64_t parse_unumber(const char* s, uint64_t min, uint64_t max, const char** errmsg) {
  errno = 0;
  char* end;
  uint64_t val = strtoull(s, &end, 0);

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
