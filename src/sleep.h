#ifndef SLEEP_H
#define SLEEP_H

#include <time.h>

static inline void sleep_ms(unsigned int milliseconds)
{
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

#endif
