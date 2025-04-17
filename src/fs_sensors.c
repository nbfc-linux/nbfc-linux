#include "fs_sensors.h"

#include "memory.h"
#include "slurp_file.h"
#include "log.h"

#include <errno.h>   // ENODATA, EINVAL
#include <stdio.h>   // snprintf
#include <string.h>  // strcmp
#include <stdbool.h> // bool
#include <stdlib.h>  // strtold
#include <linux/limits.h> // PATH_MAX

static const char* const LinuxHwmonDirs[] = {
  "/sys/class/hwmon/hwmon%d",
  "/sys/class/hwmon/hwmon%d/device",
  NULL
};

static const char* const LinuxTempSensorFile = "temp%d_input";

array_of(FS_TemperatureSource) FS_Sensors_Sources = {0};

Error* FS_TemperatureSource_GetTemperature(FS_TemperatureSource* self, float* out) {
  char buf[32];
  int nread;

  if (self->type == FS_TemperatureSource_File) {
    nread = slurp_file(buf, sizeof(buf), my.file);
  }
  else {
    FILE* fh = popen(my.file, "r");
    if (! fh)
      return err_stdlib(0, my.file);
    nread = fread(buf, 1, sizeof(buf), fh);
    int olderr = errno;
    pclose(fh);
    errno = olderr;
  }

  if (nread < 0)
    return err_stdlib(0, my.file);

  if (nread == 0)
    return (errno = ENODATA), err_stdlib(0, my.file);

  char* end;
  errno = 0;
  *out = strtold(buf, &end);
  *out *= my.multiplier;
  if (end == buf)
    errno = EINVAL;
  if (errno)
    return err_stdlib(err_string(0, buf), my.file);

  return err_success();
}

Error* FS_Sensors_Init() {
  Error* e;
  FS_TemperatureSource sources[64];
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

      for (int j = 0; j < 10; j++) {
        char filename[PATH_MAX];
        snprintf(filename, sizeof(filename), LinuxTempSensorFile, j);
        snprintf(file, sizeof(file), "%s/%s", dir, filename);

        source->name = source_name;
        source->file = file;
        source->multiplier = 0.001;
        source->type = FS_TemperatureSource_File;

        float t;
        e = FS_TemperatureSource_GetTemperature(source, &t);
#ifndef NDEBUG
        e_warn();
#endif
        if (e)
          continue;
        Log_Info("Available temperature source: '%s' (%s)\n", source->name, source->file);
        source->name = Mem_Strdup(source->name);
        source->file = Mem_Strdup(source->file);
        if (++source == sources_end)
          goto end;
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
  for_each_array(FS_TemperatureSource*, s, FS_Sensors_Sources) {
    Mem_Free(s->name);
    Mem_Free(s->file);
  }
  Mem_Free(FS_Sensors_Sources.data);
  FS_Sensors_Sources.size = 0;
  FS_Sensors_Sources.data = NULL;
}

