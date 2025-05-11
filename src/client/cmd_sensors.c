#include <string.h> // strcmp
#include <linux/limits.h> // PATH_MAX

#include "service_control.h"
#include "client_global.h"
#include "check_root.h"

#include "../nbfc.h"
#include "../memory.h"
#include "../help/client.help.h"
#include "../nxjson_utils.h"
#include "../fs_sensors.h"

/* nbfc sensors API:
 *
 * nbfc sensors show
 * nbfc sensors list
 * nbfc sensors set -f <FAN_INDEX> [-s sensor...] [-a algorithm]
 */

const cli99_option sensors_options[] = {
  cli99_include_options(&main_options),
  {"sensors_command", Option_Sensors_Command,     1 | cli99_required_option},
  cli99_options_end()
};

const cli99_option sensors_set_options[] = {
  cli99_include_options(&sensors_options),
  {"-f|--fan",        Option_Sensors_Fan,         1},
  {"-s|--sensor",     Option_Sensors_Sensor,      1},
  {"-a|--algorithm",  Option_Sensors_Algorithm,   1},
  {"--force",         Option_Sensors_Force,       0},
  cli99_options_end()
};

enum Sensors_Command {
  Sensors_Command_Set,
  Sensors_Command_List,
  Sensors_Command_Show,
  Sensors_Command_End,
};
typedef enum Sensors_Command Sensors_Command;

Sensors_Command Sensors_Command_FromString(const char* s) {
  if (! strcmp(s, "set"))    return Sensors_Command_Set;
  if (! strcmp(s, "list"))   return Sensors_Command_List;
  if (! strcmp(s, "show"))   return Sensors_Command_Show;
  return Sensors_Command_End;
}

struct {
  Sensors_Command          command;
  int                      fan;
  array_of(str)            sensors;
  TemperatureAlgorithmType algorithm;
  bool                     force;
} Sensors_Options = {
  Sensors_Command_End,
  -1,
  {NULL, 0},
  TemperatureAlgorithmType_Unset,
  false,
};

static Error* Sensors_IsValidSensor(const char* sensor) {
  switch (sensor[0]) {
    case '@':
      // TODO: Check if sensor group can be resolved to a sensor
      if (!strcmp(sensor, "@CPU"))
        return err_success();

      if (!strcmp(sensor, "@GPU"))
        return err_success();

      return err_stringf(0, "No such sensor group: %s", sensor);

    case '/':
      if (access(sensor, F_OK) == 0)
        return err_success();

      errno = ENOENT;
      return err_stdlib(0, sensor);

    case '$':
      return err_success();

    default:
      for_each_array(FS_TemperatureSource*, ts, FS_Sensors_Sources)
        if (!strcmp(ts->name, sensor))
          return err_success();

      return err_stringf(0, "No such sensor name: %s", sensor);
  }
}

static FanTemperatureSourceConfig* Sensors_GetFTSCByFanIndex(int fanIndex) {
  for_each_array(FanTemperatureSourceConfig*, ftsc, service_config.FanTemperatureSources)
    if (ftsc->FanIndex == fanIndex)
      return ftsc;

  const int idx = service_config.FanTemperatureSources.size;
  service_config.FanTemperatureSources.data = Mem_Realloc(service_config.FanTemperatureSources.data, (idx + 1) * sizeof(FanTemperatureSourceConfig));
  service_config.FanTemperatureSources.size = (idx + 1);
  return &service_config.FanTemperatureSources.data[idx];
}

static int Sensors_Set() {
  Error* e;
  ModelConfig model_config = {0};

  check_root();

  if (Sensors_Options.fan == -1) {
    Log_Error("Missing option: %s\n", "-f|--fan");
    return NBFC_EXIT_CMDLINE;
  }

  FS_Sensors_Init();
  Service_LoadAllConfigFiles(&model_config);

  if (Sensors_Options.fan >= model_config.FanConfigurations.size) {
    Log_Error("%s: No such fan: %d\n", "-f|--fan", Sensors_Options.fan);
    return NBFC_EXIT_FAILURE;
  }

  if (! Sensors_Options.force) {
    for_each_array(str*, sensor, Sensors_Options.sensors) {
      e = Sensors_IsValidSensor(*sensor);
      if (e) {
        Log_Error("%s: %s\n", "-s|--sensor", err_print_all(e));
        Log_Info("You can use --force to apply the sensor anyway\n");
        return NBFC_EXIT_FAILURE;
      }
    }
  }

  FanTemperatureSourceConfig* ftsc = Sensors_GetFTSCByFanIndex(Sensors_Options.fan);

  FanTemperatureSourceConfig_Set_FanIndex(ftsc);
  ftsc->FanIndex = Sensors_Options.fan;

  if (Sensors_Options.sensors.size) {
    FanTemperatureSourceConfig_Set_Sensors(ftsc);
    ftsc->Sensors = Sensors_Options.sensors;
  }
  else {
    FanTemperatureSourceConfig_UnSet_Sensors(ftsc);
    ftsc->Sensors.size = 0;
  }

  if (Sensors_Options.algorithm != TemperatureAlgorithmType_Unset) {
    FanTemperatureSourceConfig_Set_TemperatureAlgorithmType(ftsc);
    ftsc->TemperatureAlgorithmType = Sensors_Options.algorithm;
  }
  else {
    FanTemperatureSourceConfig_UnSet_TemperatureAlgorithmType(ftsc);
  }

  e = ServiceConfig_Write(NBFC_SERVICE_CONFIG);
  e_die();

  return NBFC_EXIT_SUCCESS;
}

static int Sensors_Show() {
  ModelConfig model_config = {0};

  Service_LoadAllConfigFiles(&model_config);

  struct FanWithTrace {
    const char*              FanName;
    array_of(str)            Sensors;
    const char*              Sensors_Source;
    TemperatureAlgorithmType TemperatureAlgorithmType;
    const char*              TemperatureAlgorithmType_Source;
  };

#define Sensors_Fan_Max 256
  struct FanWithTrace fans[Sensors_Fan_Max] = {0};

  // ==========================================================================
  // Set the defaults
  // ==========================================================================

  const char* default_sources[1] = {"@CPU"};

  for (ssize_t i = 0; i < Sensors_Fan_Max; ++i) {
    fans[i].Sensors.data = default_sources;
    fans[i].Sensors.size = 1;
    fans[i].Sensors_Source = "default";
    fans[i].TemperatureAlgorithmType = TemperatureAlgorithmType_Average;
    fans[i].TemperatureAlgorithmType_Source = "default";
  }

  // ==========================================================================
  // Override by model configuration
  // ==========================================================================

  for_enumerate_array(ssize_t, i, model_config.FanConfigurations) {
    if (i >= Sensors_Fan_Max)
      continue;

    FanConfiguration* fc = &model_config.FanConfigurations.data[i];

    fans[i].FanName = fc->FanDisplayName;

    if (FanConfiguration_IsSet_Sensors(fc)) {
      fans[i].Sensors = fc->Sensors;
      fans[i].Sensors_Source = "model config";
    }

    if (FanConfiguration_IsSet_TemperatureAlgorithmType(fc)) {
      fans[i].TemperatureAlgorithmType = fc->TemperatureAlgorithmType;
      fans[i].TemperatureAlgorithmType_Source = "model config";
    }
  }

  // ==========================================================================
  // Override by service configuration
  // ==========================================================================

  for_each_array(FanTemperatureSourceConfig*, ftsc, service_config.FanTemperatureSources) {
    if (ftsc->FanIndex >= Sensors_Fan_Max)
      continue;

    if (ftsc->FanIndex >= model_config.FanConfigurations.size)
      continue;

    if (FanTemperatureSourceConfig_IsSet_Sensors(ftsc)) {
      fans[ftsc->FanIndex].Sensors = ftsc->Sensors;
      fans[ftsc->FanIndex].Sensors_Source = "service config";
    }

    if (FanTemperatureSourceConfig_IsSet_TemperatureAlgorithmType(ftsc)) {
      fans[ftsc->FanIndex].TemperatureAlgorithmType = ftsc->TemperatureAlgorithmType;
      fans[ftsc->FanIndex].TemperatureAlgorithmType_Source = "service config";
    }
  }

  // ==========================================================================
  // Print configuration
  // ==========================================================================

  for_enumerate_array(int, i, model_config.FanConfigurations) {
    if (i >= Sensors_Fan_Max)
      continue;

    printf("#%d (%s):\n", i, fans[i].FanName);
    printf("  Sensors:  ");
    for_each_array(str*, sensor, fans[i].Sensors)
      printf(" %s", *sensor);
    printf(" (%s)\n", fans[i].Sensors_Source);
    printf("  Algorithm: %s (%s)\n",
      TemperatureAlgorithmType_ToString(fans[i].TemperatureAlgorithmType),
      fans[i].TemperatureAlgorithmType_Source);
    printf("\n");
  }

  return NBFC_EXIT_SUCCESS;
}

static int Sensors_List() {
  FS_Sensors_Init();

  const char* having[4096];
  ssize_t     having_size = 0;

  for_each_array(FS_TemperatureSource*, source, FS_Sensors_Sources) {
    bool sensor_printed = false;

    for (ssize_t i = 0; i < having_size; ++i) {
      if (! strcmp(having[i], source->name)) {
        sensor_printed = true;
        break;
      }
    }

    if (sensor_printed)
      continue;

    having[having_size++] = source->name;

    printf("%s:\n", source->name);

    for_each_array(FS_TemperatureSource*, source2, FS_Sensors_Sources) {
      if (! strcmp(source->name, source2->name))
        printf("\t%s\n", source2->file);
    }
  }

  return NBFC_EXIT_SUCCESS;
}

int Sensors() {
  switch (Sensors_Options.command) {
    case Sensors_Command_Set:    return Sensors_Set();
    case Sensors_Command_List:   return Sensors_List();
    case Sensors_Command_Show:   return Sensors_Show();
    default:
      printf("%s\n", CLIENT_SENSORS_HELP_TEXT);
      return NBFC_EXIT_FAILURE;
  }
}
