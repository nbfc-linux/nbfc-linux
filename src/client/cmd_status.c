#include "../nbfc.h"
#include "../sleep.h"
#include "../memory.h"

#include "str_functions.h"
#include "service_control.h"
#include "client_global.h"

#define CLEAR_SCREEN "\033c"

const cli99_option status_options[] = {
  cli99_include_options(&main_options),
  {"-a|--all",     Option_Status_All,     0},
  {"-s|--service", Option_Status_Service, 0},
  {"-f|--fan",     Option_Status_Fan,     1},
  {"-w|--watch",   Option_Status_Watch,   1},
  cli99_options_end()
};

struct {
  array_of(int) fans;
  bool          all;
  bool          service;
  float         watch;
} Status_Options = {0};

static void print_fan_status(const FanInfo* fan) {
  printf(
    "Fan Display Name         : %s\n"
    "Temperature              : %.2f\n"
    "Auto Control Enabled     : %s\n"
    "Critical Mode Enabled    : %s\n"
    "Current Fan Speed        : %.2f\n"
    "Target Fan Speed         : %.2f\n"
    "Requested Fan Speed      : %.2f\n"
    "Fan Speed Steps          : %d\n",
    fan->Name,
    fan->Temperature,
    bool_to_str(fan->AutoMode),
    bool_to_str(fan->Critical),
    fan->CurrentSpeed,
    fan->TargetSpeed,
    fan->RequestedSpeed,
    fan->SpeedSteps);
}

static void print_service_status(const ServiceInfo* service_info) {
  printf(
    "Read-only                : %s\n"
    "Selected Config Name     : %s\n",
    bool_to_str(service_info->ReadOnly),
    service_info->SelectedConfigId);
}

int Status() {
  ServiceInfo service_info = {0};

  if (!Status_Options.service && !Status_Options.all && !Status_Options.fans.size)
    Status_Options.all = true;

  while (true) {
    Error* e = ServiceInfo_TryLoad(&service_info);
    e_die();

    if (Status_Options.all || Status_Options.service)
      print_service_status(&service_info);

    if (Status_Options.all) {
      for_each_array(const FanInfo*, f, service_info.Fans) {
        printf("\n");
        print_fan_status(f);
      }
    }
    else if (Status_Options.fans.size) {
      const int fan_count = service_info.Fans.size;
      bool *vis = Mem_Calloc(sizeof(bool), fan_count);
      for_each_array(int*, fan_index, Status_Options.fans) {
        if (*fan_index >= fan_count) {
          Log_Error("Fan number %d not found! (Fan indexes count from zero!)\n", *fan_index);
          return NBFC_EXIT_FAILURE;
        }
        if (!vis[*fan_index]) {
          printf("\n");
          print_fan_status(&service_info.Fans.data[*fan_index]);
          vis[*fan_index] = 1;
        }
      }
      Mem_Free(vis);
    }

    if (! Status_Options.watch)
      break;

    sleep_ms(Status_Options.watch * 1000);
    printf("%s", CLEAR_SCREEN);
  }

  return NBFC_EXIT_SUCCESS;
}
