#include <errno.h>  // errno
#include <stdio.h>  // printf, snprintf
#include <string.h> // strcmp, strcspn, strerror
#include <stdbool.h>

#include "../dmi.h"
#include "../nbfc.h"
#include "../error.h"
#include "../macros.h"
#include "../log.h"
#include "../sleep.h"
#include "../file_utils.h"
#include "../fs_sensors.h"

#include "service_control.h"

static int WaitForHwmon(void) {
  const char* hwmon_file_names[] = {
    "/sys/class/hwmon/hwmon%d/name",
    "/sys/class/hwmon/hwmon%d/device/name",
    NULL
  };
  const char* linux_temp_sensor_names[] = {
    "coretemp", "k10temp", "zenpower", NULL
  };

  char filename[1024];
  char content[1024];

  for (int tries = 0; tries < 30; tries++) {
    for (const char** format = hwmon_file_names; *format; ++format) {
      for (int i = 0; i < 10; i++) {
        snprintf(filename, sizeof(filename), *format, i);
        if (! File_Read(content, sizeof(content), filename).ok)
          continue;

        // trim the newline
        content[strcspn(content, "\n")] = '\0';
        for (const char** sensor_name = linux_temp_sensor_names; *sensor_name; ++sensor_name) {
          if (!strcmp(content, *sensor_name)) {
            printf("Success!\n");
            return NBFC_EXIT_SUCCESS;
          }
        }
      }
    }
    sleep_ms(1000);
  }

  return NBFC_EXIT_FAILURE;
}

static int GetModelName(void) {
  Error e;
  char model_name[DMI_MAX_MODEL_NAME_LEN];

  e = DMI_GetModelName(model_name, sizeof(model_name));
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  printf("%s\n", model_name);
  return NBFC_EXIT_SUCCESS;
}

static int CompleteFans(void) {
  ServiceConfig service_config = {0};
  ModelConfig model_config = {0};

  Log_LogLevel = LogLevel_Quiet;

  Service_LoadAllConfigFiles(&service_config, &model_config);

  int idx = 0;
  for_each_array(const FanConfiguration*, fc, model_config.FanConfigurations)
    printf("%d\t%s\n", idx++, fc->FanDisplayName);

#if STRICT_CLEANUP
  ServiceConfig_Free(&service_config);
  ModelConfig_Free(&model_config);
#endif

  return NBFC_EXIT_SUCCESS;
}

static int CompleteSensors(void) {
  Error e;
  bool source_was_printed[32768] = {0};
  FS_TemperatureSource_References found_sources = {0};

  Log_LogLevel = LogLevel_Quiet;

  e = FS_Sensors_Init(true);
  if (e)
    return NBFC_EXIT_SUCCESS; // Success is intentional

  e = FS_TemperatureSources_AddTemperatureSources(&found_sources, "@CPU");
  if (! e)
    printf("%s\t%s\n", "@CPU", "group");
  Mem_Free(found_sources.data);
  memset(&found_sources, 0, sizeof(found_sources));

  e = FS_TemperatureSources_AddTemperatureSources(&found_sources, "@GPU");
  if (! e)
    printf("%s\t%s\n", "@GPU", "group");
  Mem_Free(found_sources.data);
  memset(&found_sources, 0, sizeof(found_sources));

  for_each_array(FS_TemperatureSource*, source, FS_Sensors_Sources) {
    if (source_was_printed[source - FS_Sensors_Sources.data])
      continue;

    printf("%s\tsensor\n", source->name);

    for_each_array(FS_TemperatureSource*, source2, FS_Sensors_Sources) {
      if (! strcmp(source->name, source2->name)) {
        source_was_printed[source2 - FS_Sensors_Sources.data] = true;
      }
    }
  }

#if STRICT_CLEANUP
  FS_Sensors_Cleanup();
#endif

  return NBFC_EXIT_SUCCESS;
}

static int FAQ(void) {
  execlp("man", "man", "nbfc.faq", NULL);
  Log_Error("execlp(): %s", strerror(errno));
  return NBFC_EXIT_FAILURE;
}
