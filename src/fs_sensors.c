#include "fs_sensors.h"

#include "macros.h"
#include "memory.h"

#include <string.h>
#include <stdio.h>
#include <limits.h> // PATH_MAX

static const char* const LinuxHwmonDirs[] = {
  "/sys/class/hwmon/hwmon%d",
  "/sys/class/hwmon/hwmon%d/device",
  NULL
};

static const char* const LinuxTempSensorNames[] = {
  "coretemp",
  "k10temp",
  "zenpower",
  NULL
};

static const char* const LinuxTempSensorFile = "temp%d_input";

typedef struct FS_TemperatureSource FS_TemperatureSource;
struct FS_TemperatureSource {
  const char* file;
  float multiplier;
};

declare_array_of(FS_TemperatureSource);

static array_of(FS_TemperatureSource) FS_Sensors_Sources;

static Error* FS_TemperatureSource_GetTemperature(FS_TemperatureSource* self, float* out) {
  char buf[32];
  int nread = slurp_file(buf, sizeof(buf) - 1, my.file);
  if (nread < 0)  return err_stdlib(0, my.file);
  if (nread == 0) return (errno = EINVAL), err_stdlib(0, my.file);

  char* end;
  errno = 0;
  *out = strtold(buf, &end);
  *out *= my.multiplier;
  if (end == buf)
    errno = EINVAL;
  if (errno)
    return err_stdlib(0, Temp_Strdup(buf));

  return err_success();
}


Error* FS_Sensors_GetTemperature(float* out) {
  Error* e = NULL;
  float tmp, sum = 0;
  int   total = 0;
  for_each_array(FS_TemperatureSource*, s, FS_Sensors_Sources) {
    e = FS_TemperatureSource_GetTemperature(s, &tmp);
    if (! e) {
      sum += tmp;
      total++;
    }
  }

  if (! total)
    return err_string(0, "No temperatures available");
  *out = sum / total;
  return err_success();
}

Error* FS_Sensors_Init() {
  Error* e;
  char dir[PATH_MAX];
  char file[PATH_MAX * 2];
  FS_TemperatureSource sources[32];
  FS_TemperatureSource *source = sources;
  FS_TemperatureSource *const sources_end = &sources[32];
  int n_sources;

  for (const char* const* hwmonDir = LinuxHwmonDirs; *hwmonDir; ++hwmonDir) {
    for (int i = 0; i < 10; i++) {
      snprintf(dir,  sizeof(dir), *hwmonDir, i);
      snprintf(file, sizeof(file), "%s/%s", dir, "name");

      char sourceName[256];
      int nread = slurp_file(sourceName, sizeof(sourceName), file);
      if (nread < 0) {
        if (errno != ENOENT) {
          e = err_stdlib(0, file);
          e_warn();
        }
        continue;
      }

      while (nread && sourceName[nread] < 32)
        sourceName[nread--] = '\0'; /* strip whitespace */

      for (const char* const* n = LinuxTempSensorNames; *n; ++n) {
        if (strcmp(sourceName, *n))
          continue;

        for (int j = 0; j < 10; j++) {
          snprintf(dir, sizeof(dir), "%s/%s", *hwmonDir, LinuxTempSensorFile);
          snprintf(file, sizeof(file), dir, i, j);

          source->file = file;
          source->multiplier = 0.001;

          float t;
          e = FS_TemperatureSource_GetTemperature(source, &t);
#ifndef NDEBUG
          e_warn();
#endif
          if (! e) {
            fprintf(stderr, "Using '%s' as temperature source\n", file);
            source->file = Mem_Strdup(file);
            if (++source == sources_end)
              goto end;
          }
        }
      }
    }
  }

end:
  n_sources = source - sources;
  if (! n_sources)
    return err_string(0, "No temperature sources found");

  FS_Sensors_Sources.size = n_sources;
  FS_Sensors_Sources.data = (FS_TemperatureSource*) Mem_Malloc(n_sources, sizeof(FS_TemperatureSource));
  memcpy(FS_Sensors_Sources.data, sources, n_sources * sizeof(FS_TemperatureSource));
  return err_success();
}

void FS_Sensors_Cleanup() {

}

Sensor_VTable FS_Sensors_VTable = {
  FS_Sensors_Init,
  FS_Sensors_Cleanup,
  FS_Sensors_GetTemperature
};

