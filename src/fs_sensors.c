#include "fs_sensors.h"

#include "memory.h"
#include "buffer.h"
#include "file_utils.h"
#include "log.h"
#include "sleep.h"
#include "nvidia.h"
#include "str_functions.h"
#include "vfio.h"

#include <float.h>   // FLT_MAX
#include <errno.h>   // ENODATA, EINVAL
#include <stdio.h>   // snprintf
#include <stdlib.h>  // strtod
#include <string.h>  // strstr
#include <linux/limits.h> // PATH_MAX

static inline bool IsCPUSensorName(const char* s) {
  return
    !strcmp(s, "coretemp") ||
    !strcmp(s, "k10temp")  ||
    !strcmp(s, "zenpower");
}

static inline bool IsGPUSensorName(const char* s) {
  return
    !strcmp(s, "amdgpu")    ||
    !strcmp(s, "nvidia")    ||
    !strcmp(s, "nvidia-ml") ||
    !strcmp(s, "nouveau")   ||
    !strcmp(s, "radeon");
}

static const char* const LinuxHwmonDirs[] = {
  "/sys/class/hwmon/hwmon%d",
  "/sys/class/hwmon/hwmon%d/device",
  NULL
};

static const char* const LinuxTempSensorFile = "temp%d_input";

array_of(FS_TemperatureSource) FS_Sensors_Sources = {0};

Error FS_TemperatureSource_GetTemperature(const FS_TemperatureSource* self, float* out) {
  char buf[32];
  FileResult res;
  res.ok = true;

  if (self->type == FS_TemperatureSource_File) {
    res = File_Read(buf, sizeof(buf), my.file);
  }
  else if (self->type == FS_TemperatureSource_Nvidia) {
    return Nvidia_GetTemperature(out);
  }
  else {
    FILE* fh = popen(my.file, "r");
    if (! fh)
      return err_stdlib(my.file);
    res.len = fread(buf, 1, sizeof(buf), fh);
    int olderr = errno;
    pclose(fh);
    errno = olderr;
  }

  if (! res.ok)
    return err_stdlib(my.file);

  if (res.len == 0) {
    errno = ENODATA;
    return err_stdlib(my.file);
  }

  char* end;
  errno = 0;
  *out = (float) strtod(buf, &end);
  *out *= my.multiplier;
  if (end == buf)
    errno = EINVAL;
  if (errno)
    return err_chain_stdlib(err_string(buf), my.file);

  return err_success();
}

#define FS_SENSORS_MAX_SOURCES 256
#define FS_SENSORS_BUFFER_SIZE (sizeof(FS_TemperatureSource) * FS_SENSORS_MAX_SOURCES)

static Error FS_Sensors_Init_HwMon(void) {
  Error e;
  char* dir = Buffer_Get(PATH_MAX);
  char* file = Buffer_Get(PATH_MAX);
  char* filename = Buffer_Get(PATH_MAX);
  FS_TemperatureSource* sources = (FS_TemperatureSource*) Buffer_Get(FS_SENSORS_BUFFER_SIZE);
  array_size_t n_sources = 0;

  for (const char* const* hwmonDir = LinuxHwmonDirs; *hwmonDir; ++hwmonDir) {
    for (int i = 0; i < 20; i++) {
      snprintf(dir,  PATH_MAX, *hwmonDir, i);
      snprintf(file, PATH_MAX, "%s/name", dir);

      char source_name[256];
      FileResult res = File_Read(source_name, sizeof(source_name), file);
      if (! res.ok) {
        if (errno != ENOENT) {
          e = err_stdlib(file);
          e_warn();
        }
        continue;
      }

      str_rstrip_whitespace(source_name, res.len);

      for (int j = 0; j < 10; j++) {
        if (n_sources >= FS_SENSORS_MAX_SOURCES)
          goto end;

        snprintf(filename, PATH_MAX, LinuxTempSensorFile, j);
        snprintf(file, PATH_MAX, "%s/%s", dir, filename);

        FS_TemperatureSource* source = &sources[n_sources];
        source->name = source_name;
        source->file = file;
        source->multiplier = 0.001f;
        source->type = FS_TemperatureSource_File;

        float t;
        e = FS_TemperatureSource_GetTemperature(source, &t);
#ifndef NDEBUG
        e_warn();
#endif
        if (e)
          continue;

        ++n_sources;
        source->name = Mem_Strdup(source->name);
        source->file = Mem_Strdup(source->file);
      }
    }
  }

end:
  Buffer_Release(dir, PATH_MAX);
  Buffer_Release(file, PATH_MAX);
  Buffer_Release(filename, PATH_MAX);

  if (! n_sources) {
    Buffer_Release((char*) sources, FS_SENSORS_BUFFER_SIZE);
    return err_string("No temperature sources found");
  }

  FS_Sensors_Sources.size = n_sources;
  array_calloc(FS_TemperatureSource, FS_Sensors_Sources, n_sources);
  memcpy(FS_Sensors_Sources.data, sources, n_sources * sizeof(FS_TemperatureSource));
  Buffer_Release((char*) sources, FS_SENSORS_BUFFER_SIZE);
  return err_success();
}

void FS_Sensors_Log(void) {
  for_each_array(FS_TemperatureSource*, source, FS_Sensors_Sources)
    Log_Info("Available temperature source: \"%s\" (%s)", source->name, source->file);
}

// ============================================================================
// FS_Sensors_Init / FS_Sensors_Cleanup
// ============================================================================

Error FS_Sensors_Init(void) {
  Error e;
  int slept;
  const int sleep_time = 30;

  // Wait for /sys/class/hwmon/* sensors
  for (slept = 0; slept < sleep_time; ++slept) {
    e = FS_Sensors_Init_HwMon();
    if (! e)
      break;
    Log_Info("Waiting for /sys/class/hwmon* sensors ...");
    sleep_ms(1000);
  }

  // If VFIO passthrough is active, skip nvidia-ml entirely.
  // Checking /proc/cmdline alone is not enough because users may
  // configure passthrough via /etc/modprobe.d/, driverctl, etc.
  if (!VFIO_CheckProcCmdline() && !VFIO_CheckSysBusPciDevices()) {
    // Wait for nvidia module
    for (; slept < sleep_time; ++slept) {
      Nvidia_Error ne = Nvidia_Init();
      if (ne == Nvidia_Error_DlOpen)
        break;

      if (ne == Nvidia_Error_API) {
        Log_Info("Waiting for nvidia sensor ...");
        sleep_ms(1000);
        continue;
      }

      const array_size_t idx = FS_Sensors_Sources.size;
      array_realloc(FS_TemperatureSource, FS_Sensors_Sources, (idx + 1));
      FS_Sensors_Sources.data[idx].name = Mem_Strdup("nvidia-ml");
      FS_Sensors_Sources.data[idx].file = Mem_Strdup("none");
      FS_Sensors_Sources.data[idx].multiplier = 1;
      FS_Sensors_Sources.data[idx].type = FS_TemperatureSource_Nvidia;
      FS_Sensors_Sources.size = idx + 1;
      break;
    }
  }

  if (! FS_Sensors_Sources.size)
    return err_string("No temperature sources found");

  return err_success();
}

void FS_Sensors_Cleanup(void) {
  Nvidia_Close();

  for_each_array(FS_TemperatureSource*, s, FS_Sensors_Sources) {
    Mem_Free(s->name);
    Mem_Free(s->file);
  }
  Mem_Free(FS_Sensors_Sources.data);
  FS_Sensors_Sources.size = 0;
  FS_Sensors_Sources.data = NULL;
}

Error FS_TemperatureSources_GetTemperature(
  FS_TemperatureSource_References* sources,
  TemperatureAlgorithmType algorithm,
  float* out)
{
  float tmp;
  float sum = 0;
  float min = FLT_MAX;
  float max = FLT_MIN;
  int   total = 0;

  for_each_array(array_size_t*, ts_idx, *sources) {
    const FS_TemperatureSource* ts = FS_Sensors_Sources_UnRef(*ts_idx);
    Error e = FS_TemperatureSource_GetTemperature(ts, &tmp);
    e_warn();
    if (! e) {
      min = MIN(min, tmp);
      max = MAX(max, tmp);
      sum += tmp;
      ++total;
    }
  }

  if (! total)
    return err_string("No temperatures available");

  switch (algorithm) {
    case TemperatureAlgorithmType_Average:
      *out = sum / (float) total;
      return err_success();
    case TemperatureAlgorithmType_Min:
      *out = min;
      return err_success();
    case TemperatureAlgorithmType_Max:
      *out = max;
      return err_success();
    default:
      return err_string("ERR-03");
  }
}

void FS_TemperatureSources_AddTemperatureSource(
  FS_TemperatureSource_References* out,
  size_t ref_idx)
{
  const size_t idx = out->size;
  array_realloc(array_size_t, *out, (idx + 1));
  out->data[idx] = ref_idx;
  out->size++;
}

// Adds one or more FS_TemperatureSource_Ptr to an array.
//
// If `sensor` is not found in `FS_Sensors_Sources` by its name or its path,
// this function assumes that `sensor` is a user defined file path to a file
// containing the temperature.
//
// Return error if `sensor` is not found in available temperature sources
// or `sensor` is not a valid file path to a temperature file.
Error FS_TemperatureSources_AddTemperatureSources(
  FS_TemperatureSource_References* out,
  const char* sensor)
{
  Error e;
  bool found_sensors = false;

  // ==========================================================================
  // Sensor group "@CPU": Add all sensors found in `IsCPUSensorName`
  // ==========================================================================
  if (!strcmp(sensor, "@CPU")) {
    for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
      if (IsCPUSensorName(ts->name)) {
        FS_TemperatureSources_AddTemperatureSource(out, FS_Sensors_Sources_Ref(ts));
        found_sensors = true;
      }
    }

    return found_sensors
      ? err_success()
      : err_stringf("%s: No sensors found", "@CPU");
  }

  // ==========================================================================
  // Sensor group "@GPU": Add all sensors found in `IsGPUSensorName`
  // ==========================================================================
  if (!strcmp(sensor, "@GPU")) {
    for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
      if (IsGPUSensorName(ts->name)) {
        FS_TemperatureSources_AddTemperatureSource(out, FS_Sensors_Sources_Ref(ts));
        found_sensors = true;
      }
    }

    return found_sensors
      ? err_success()
      : err_stringf("%s: No sensors found", "@GPU");
  }

  // ==========================================================================
  // Add sensors by name or path (for available sensors)
  // ==========================================================================
  for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources) {
    if (!strcmp(sensor, ts->name) || !strcmp(sensor, ts->file)) {
      FS_TemperatureSources_AddTemperatureSource(out, FS_Sensors_Sources_Ref(ts));
      found_sensors = true;
    }
  }

  if (found_sensors)
    return err_success();

  // ==========================================================================
  // Create a new TemperatureSource (a user defined file or command)
  // ==========================================================================
  FS_TemperatureSource source;

  if (sensor[0] == '$') {
    // Sensor is a command
    source.name = "command";
    source.file = (char*) sensor + 1;
    source.type = FS_TemperatureSource_Command;
    source.multiplier = 1;
  }
  else {
    // Sensor is a user defined file
    source.name = "anonymous";
    source.file = (char*) sensor;
    source.type = FS_TemperatureSource_File;
    source.multiplier = 0.001f;
  }

  float t; // NOLINT
  e = FS_TemperatureSource_GetTemperature(&source, &t);
  if (e)
    return e;

  const array_size_t idx = FS_Sensors_Sources.size;
  array_realloc(FS_TemperatureSource, FS_Sensors_Sources, (idx + 1));
  FS_Sensors_Sources.data[idx].name = Mem_Strdup(source.name);
  FS_Sensors_Sources.data[idx].file = Mem_Strdup(source.file);
  FS_Sensors_Sources.data[idx].multiplier = source.multiplier;
  FS_Sensors_Sources.data[idx].type = source.type;
  FS_Sensors_Sources.size = idx + 1;
  FS_TemperatureSources_AddTemperatureSource(out, idx);

  return err_success();
}
