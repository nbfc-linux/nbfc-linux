#include "../nbfc.h"
#include "../sleep.h"
#include "../memory.h"

#include "str_functions.h"
#include "service_control.h"
#include "client_global.h"

#define STATUS_CLEAR_SCREEN "\033c"

const struct cli99_Option status_options[] = {
  cli99_Options_Include(&main_options),
  {"-a|--all",     Option_Status_All,     cli99_NoArgument      },
  {"-s|--service", Option_Status_Service, cli99_NoArgument      },
  {"-f|--fan",     Option_Status_Fan,     cli99_RequiredArgument},
  {"-w|--watch",   Option_Status_Watch,   cli99_RequiredArgument},
  cli99_Options_End()
};

struct {
  array_of(array_size_t) fans;
  bool                   all;
  bool                   service;
  float                  watch;
} Status_Options = {0};

static void Status_Print_Fan(const FanInfo* fan) {
  printf(
    "Fan Display Name         : %s\n"
    "Temperature              : %.2f\n"
    "Auto Control Enabled     : %s\n"
    "Critical Mode Enabled    : %s\n"
    "Current Fan Speed        : %.2f\n"
    "Target Fan Speed         : %.2f\n"
    "Fan Speed Steps          : %d\n",
    fan->Name,
    (double) fan->Temperature,
    str_from_bool(fan->AutoMode),
    str_from_bool(fan->Critical),
    (double) fan->CurrentSpeed,
    (double) fan->TargetSpeed,
    fan->SpeedSteps);
}

static void Status_Print_Service(const ServiceInfo* service_info) {
  printf(
    "Read-only                : %s\n"
    "Selected Config Name     : %s\n",
    str_from_bool(service_info->ReadOnly),
    service_info->SelectedConfigId);
}

static void Status_Print(void) {
  Error e;
  ServiceInfo service_info = {0};

  e = ServiceInfo_TryLoad(&service_info);
  e_die();

  if (Status_Options.all || Status_Options.service)
    Status_Print_Service(&service_info);

  if (Status_Options.all) {
    for_each_array(const FanInfo*, f, service_info.Fans) {
      printf("\n");
      Status_Print_Fan(f);
    }
  }
  else if (Status_Options.fans.size) {
    const array_size_t fan_count = service_info.Fans.size;
    for_each_array(array_size_t*, fan_index, Status_Options.fans) {
      if (*fan_index >= fan_count) {
        e = err_stringf("Fan number %zd not found! (Fan indexes count from zero!)", *fan_index);
        e_die();
      }

      printf("\n");
      Status_Print_Fan(&service_info.Fans.data[*fan_index]);
    }
  }
}

int Status(void) {
  if (!Status_Options.service && !Status_Options.all && !Status_Options.fans.size)
    Status_Options.all = true;

  while (true) {
    Status_Print();

    if (! Status_Options.watch)
      break;

    sleep_ms((unsigned) (Status_Options.watch * 1000));
    printf("%s", STATUS_CLEAR_SCREEN);
  }

  return NBFC_EXIT_SUCCESS;
}
