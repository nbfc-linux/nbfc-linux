#include "temperature_filter.h"

#include "nbfc.h"
#include "memory.h"

#include <math.h>

Error* TemperatureFilter_Init(TemperatureFilter* self, int poll_interval, int timespan) {
  if (poll_interval <= 0)
    return (errno = EINVAL), err_stdlib(0, "poll_interval");

  if (timespan <= 0)
    return (errno = EINVAL), err_stdlib(0, "timespan");

  my.index = 0;
  my.ring_buffer.size = timespan / poll_interval + !!(timespan % poll_interval);
  my.ring_buffer.data = (float*) Mem_Calloc(my.ring_buffer.size, sizeof(float));
  my.buffer_is_full = false;
  return err_success();
}

float TemperatureFilter_FilterTemperature(TemperatureFilter* self, float temperature) {
  my.sum -= my.ring_buffer.data[my.index];
  my.sum += temperature;
  my.ring_buffer.data[my.index] = temperature;

  if (++my.index == my.ring_buffer.size) {
    my.index = 0;
    my.buffer_is_full = true;
  }

  return my.sum / (my.buffer_is_full ? my.ring_buffer.size : my.index);
}
