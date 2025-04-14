#include <stdio.h>  // printf, snprintf
#include <string.h> // strcmp, strcspn

#include "../nbfc.h"
#include "../macros.h"
#include "../sleep.h"
#include "../slurp_file.h"

#include "dmi.h"
#include "service_control.h"

static int Wait_For_Hwmon() {
  const char *hwmon_file_names[] = {
    "/sys/class/hwmon/hwmon%d/name",
    "/sys/class/hwmon/hwmon%d/device/name",
    NULL
  };
  const char *linux_temp_sensor_names[] = {
    "coretemp", "k10temp", "zenpower", NULL
  };

  char filename[1024];
  char content[1024];

  for (int tries = 0; tries < 30; tries++) {
    for (const char** format = hwmon_file_names; *format; ++format) {
      for (int i = 0; i < 10; i++) {
        snprintf(filename, sizeof(filename), *format, i);
        if (slurp_file(content, sizeof(content), filename) == -1)
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

static int Get_Model_Name() {
  printf("%s\n", get_model_name());
  return NBFC_EXIT_SUCCESS;
}

static int Complete_Fans() {
  ServiceInfo service_info = {0};

  Error* e = ServiceInfo_TryLoad(&service_info);
  if (e)
    return NBFC_EXIT_FAILURE;

  int idx = 0;
  for_each_array(const FanInfo*, f, service_info.Fans)
    printf("%d\t%s\n", idx++, f->Name);

  return NBFC_EXIT_SUCCESS;
}
