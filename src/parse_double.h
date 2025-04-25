#ifndef PARSE_DOUBLE_H_
#define PARSE_DOUBLE_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static double parse_double(const char* s, double min, double max, const char** errmsg) {
  errno = 0;
  char* end;
  double val = strtold(s, &end);

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
