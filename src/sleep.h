#ifndef NBFC_SLEEP_H_
#define NBFC_SLEEP_H_

#include <time.h>

/*
 * EINTR is intentionally ignored here. If interrupted by a signal, we do not
 * want to resume sleeping, but continue as quickly as possible to the code
 * that handles the signal, e.g. by checking the `quit` variable.
 */
static inline void sleep_ms(unsigned int milliseconds)
{
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

#endif
