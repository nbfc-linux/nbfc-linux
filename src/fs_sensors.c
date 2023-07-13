#include "fs_sensors.h"

#include "macros.h"
#include "memory.h"
#include "slurp_file.h"
#include "log.h"

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

struct FS_TemperatureSource {
  char* file;
  float multiplier;
};
typedef struct FS_TemperatureSource FS_TemperatureSource;
declare_array_of(FS_TemperatureSource);

static array_of(FS_TemperatureSource) FS_Sensors_Sources = {0};

static Error* FS_TemperatureSource_GetTemperature(FS_TemperatureSource* self, float* out) {
  static char buf[32];
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
    return err_stdlib(0, buf);

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
  FS_TemperatureSource sources[32];
  FS_TemperatureSource *source = sources;
  FS_TemperatureSource *const sources_end = &sources[ARRAY_SIZE(sources)];
  char dir[PATH_MAX];
  char file[PATH_MAX];
  int n_sources;

  for (const char* const* hwmonDir = LinuxHwmonDirs; *hwmonDir; ++hwmonDir) {
    for (int i = 0; i < 10; i++) {
      snprintf(dir,  sizeof(dir), *hwmonDir, i);
      snprintf(file, sizeof(file), "%s/name", dir);

      char source_name[256];
      int nread = slurp_file(source_name, sizeof(source_name), file);
      if (nread < 0) {
        if (errno != ENOENT) {
          e = err_stdlib(0, file);
          e_warn();
        }
        continue;
      }

      while (nread && source_name[nread] < 32)
        source_name[nread--] = '\0'; /* strip whitespace */

      for (const char* const* n = LinuxTempSensorNames; *n; ++n) {
        if (strcmp(source_name, *n))
          continue;

        for (int j = 0; j < 10; j++) {
          char filename[PATH_MAX];
          snprintf(filename, sizeof(filename), LinuxTempSensorFile, j);
          snprintf(file, sizeof(file), "%s/%s", dir, filename);

          source->file = file;
          source->multiplier = 0.001;

          float t;
          e = FS_TemperatureSource_GetTemperature(source, &t);
#ifndef NDEBUG
          e_warn();
#endif
          if (! e) {
            Log_Info("Using '%s' as temperature source\n", file);
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
  FS_Sensors_Sources.data = (FS_TemperatureSource*) Mem_Malloc(n_sources * sizeof(FS_TemperatureSource));
  memcpy(FS_Sensors_Sources.data, sources, n_sources * sizeof(FS_TemperatureSource));
  return err_success();
}

void FS_Sensors_Cleanup() {
  if (FS_Sensors_Sources.size) {
    for_each_array(FS_TemperatureSource*, s, FS_Sensors_Sources)
      Mem_Free(s->file);
    Mem_Free(FS_Sensors_Sources.data);
    FS_Sensors_Sources.size = 0;
    FS_Sensors_Sources.data = NULL;
  }
}

Sensor_VTable FS_Sensors_VTable = {
  FS_Sensors_Init,
  FS_Sensors_Cleanup,
  FS_Sensors_GetTemperature
};

