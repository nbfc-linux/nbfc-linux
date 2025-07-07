#define _XOPEN_SOURCE 500 /* unistd.h: export pwrite()/pread(), string.h: export strdup */

#include <string.h>
#include <locale.h>
#include <getopt.h>
#include <limits.h>

#include "ec.h"
#include "nbfc.h"
#include "log.c"
#include "error.c"
#include "file_utils.c"
#include "trace.c"
#include "memory.c"
#include "nxjson.c"
#include "model_config.c"
#include "program_name.c"
#include "fan.c"
#include "temperature_threshold_manager.c"
#include "stack_memory.c"

EC_VTable* ec;

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
    Log_Error("Missing file\n");
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

  Log_Info(">>> Processing %s ...\n", file);

  Error* e = ModelConfig_FromFile(&model_config, path);
  e_die();

  Trace trace = {0};
  e = ModelConfig_Validate(&trace, &model_config);
  if (e) {
    e = err_string(e, trace.buf);
    Log_Error("%s\n", err_print_all(e));
    ret = 1;
    goto end;
  }

  TemperatureThresholdManager_LegacyBehaviour = model_config.LegacyTemperatureThresholdsBehaviour;
  Log_Info("TemperatureThresholdManager_LegacyBehaviour = %s\n",
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
        Log_Info("[%ld]: temp = %3d, speed = %f\n", i, temp, speed);
      seen_0_speed   |= (speed == 0.0f);
      seen_100_speed |= (speed == 100.0f);
    }

    if (! seen_0_speed && seen_0_threshold) {
      Log_Error("%s[%ld]: Didn't see 0.0 speed\n", file, i);
      ret = 1;
      goto end;
    }

    if (! seen_100_speed) {
      Log_Error("%s[%ld]: Didn't see 100.0 speed\n", file, i);
      ret = 1;
      goto end;
    }

    seen_0_speed   = false;
    seen_100_speed = false;
    for (int temp = 100; temp >= 0; --temp) {
      Fan_SetTemperature(&fan, temp);
      float speed = Fan_GetTargetSpeed(&fan);
      if (options.verbose)
        Log_Info("[%ld]: temp = %3d, speed = %f\n", i, temp, speed);
      seen_0_speed   |= (speed == 0.0f);
      seen_100_speed |= (speed == 100.0f);
    }

    if (! seen_0_speed && seen_0_threshold) {
      Log_Error("%s[%ld]: Didn't see 0.0 speed\n", file, i);
      ret = 1;
      goto end;
    }

    if (! seen_100_speed) {
      Log_Error("%s[%ld]: Didn't see 100.0 speed\n", file, i);
      ret = 1;
      goto end;
    }
  }

end:
  ModelConfig_Free(&model_config);
  return ret;
}
