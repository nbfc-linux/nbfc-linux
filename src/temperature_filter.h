#ifndef NBFC_TEMPERATURE_FILTER_H_
#define NBFC_TEMPERATURE_FILTER_H_

#include "macros.h"
#include "error.h"

#include <stdlib.h>
#include <stdbool.h>

typedef struct TemperatureFilter TemperatureFilter;
struct TemperatureFilter {
  float           sum;
  array_of(float) ring_buffer;
  ssize_t         index;
  bool            buffer_is_full;
};

Error* TemperatureFilter_Init(TemperatureFilter*, int poll_interval, int timespan);
float  TemperatureFilter_FilterTemperature(TemperatureFilter*, float temperature);
void   TemperatureFilter_Close(TemperatureFilter*);

#endif
