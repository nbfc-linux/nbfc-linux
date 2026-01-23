// The data structures returned by nxjson are temporary and are loaded into proper C structs.
// We allocate memory from a pool to avoid malloc() and reduce memory usage.
#define NX_JSON_CALLOC(SIZE) ((nx_json*) NXJSON_Memory_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (NXJSON_Memory_Free((void*) (JSON)))

#define _XOPEN_SOURCE 500 // unistd.h: export pwrite()/pread()

#include <string.h>
#include <locale.h>
#include <getopt.h>
#include <limits.h>

#include "ec.h"
#include "nbfc.h"
#include "buffer.c"
#include "log.c"
#include "error.c"
#include "file_utils.c"
#include "trace.c"
#include "memory.c"
#include "nxjson_memory.c"
#include "nxjson.c"
#include "model_config.c"
#include "program_name.c"
#include "fan.c"
#include "acpi_call.c"
#include "process.c"
#include "temperature_threshold_manager.c"

const EC_VTable* ec;

int test_model_config(const char*);

static struct option long_options[] = {
  {"verbose", no_argument, 0, 'v'},
  {0,         0,           0,  0 },
};

static const char options_str[] = "v";

static struct {
  int verbose;
} options = {0};

int main(int argc, char** argv) {
  Program_Name_Set(argv[0]);
  setlocale(LC_NUMERIC, "C"); // for json floats

  int o, option_index;
  while ((o = getopt_long(argc, argv, options_str, long_options, &option_index)) != -1) {
    switch (o) {
    case 'v': options.verbose = 1; break;
    default:  return NBFC_EXIT_CMDLINE;
    }
  }

  if (optind >= argc) {
    Log_Error("Missing file");
    return NBFC_EXIT_CMDLINE;
  }

  int ret = 0;

  while (optind < argc) {
    ret |= test_model_config(argv[optind++]);
  }

  return ret;
}

int test_model_config(const char* file) {
  int ret = 0;
  char path[PATH_MAX];
  ModelConfig model_config = {0};

  if (strchr(file, '/'))
    snprintf(path, PATH_MAX, "%s", file);
  else
    snprintf(path, PATH_MAX, "%s/%s.json", NBFC_MODEL_CONFIGS_DIR, file);

  Log_Info(">>> Processing %s ...", file);

  Error e = ModelConfig_FromFile(&model_config, path);
  e_die();

  Trace trace = {0};
  e = ModelConfig_Validate(&trace, &model_config);
  if (e) {
    e = err_chain_string(e, trace.buf);
    Log_Error("%s", err_print_all(e));
    ret = 1;
    goto end;
  }

  TemperatureThresholdManager_LegacyBehaviour = model_config.LegacyTemperatureThresholdsBehaviour;
  Log_Info("TemperatureThresholdManager_LegacyBehaviour = %s",
    TemperatureThresholdManager_LegacyBehaviour ? "true" : "false");

  for_enumerate_array(ssize_t, i, model_config.FanConfigurations) {
    Fan fan;

    e = Fan_Init(
        &fan,
        &model_config.FanConfigurations.data[i],
        &model_config
    );
    e_die();

    bool seen_0_threshold = false;

    for_each_array(TemperatureThreshold*, t, model_config.FanConfigurations.data[i].TemperatureThresholds) {
      if (t->FanSpeed == 0) {
        seen_0_threshold = true;
        break;
      }
    }

    bool seen_0_speed, seen_100_speed;

    seen_0_speed   = false;
    seen_100_speed = false;
    for (int temp = 0; temp <= 100; ++temp) {
      Fan_SetTemperature(&fan, temp);
      float speed = Fan_GetTargetSpeed(&fan);
      if (options.verbose)
        Log_Info("[%ld]: temp = %3d, speed = %f", i, temp, speed);
      seen_0_speed   |= (speed == 0.0f);
      seen_100_speed |= (speed == 100.0f);
    }

    if (! seen_0_speed && seen_0_threshold) {
      Log_Error("%s[%ld]: Didn't see 0.0 speed", file, i);
      ret = 1;
      goto end;
    }

    if (! seen_100_speed) {
      Log_Error("%s[%ld]: Didn't see 100.0 speed", file, i);
      ret = 1;
      goto end;
    }

    seen_0_speed   = false;
    seen_100_speed = false;
    for (int temp = 100; temp >= 0; --temp) {
      Fan_SetTemperature(&fan, temp);
      float speed = Fan_GetTargetSpeed(&fan);
      if (options.verbose)
        Log_Info("[%ld]: temp = %3d, speed = %f", i, temp, speed);
      seen_0_speed   |= (speed == 0.0f);
      seen_100_speed |= (speed == 100.0f);
    }

    if (! seen_0_speed && seen_0_threshold) {
      Log_Error("%s[%ld]: Didn't see 0.0 speed", file, i);
      ret = 1;
      goto end;
    }

    if (! seen_100_speed) {
      Log_Error("%s[%ld]: Didn't see 100.0 speed", file, i);
      ret = 1;
      goto end;
    }
  }

end:
  ModelConfig_Free(&model_config);
  return ret;
}
