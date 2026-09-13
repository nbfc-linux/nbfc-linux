#ifndef NBFC_TO_BINARY_H_
#define NBFC_TO_BINARY_H_

#include <stdint.h>
#include <string.h> // memset

const char* to_binary(uint64_t val, unsigned pad)
{
  static char buf[65];
  unsigned i = 63;

  memset(buf, '0', 64);
  buf[64] = '\0';

  while (val) {
    buf[i--] = '0' + (val & 1);
    val >>= 1;
  }

  unsigned first = i + 1;
  if (first > 64 - pad)
    first = 64 - pad;

  return &buf[first];
}

#endif
