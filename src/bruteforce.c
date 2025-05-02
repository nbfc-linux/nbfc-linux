#define _XOPEN_SOURCE  500 // unistd.h: export pwrite()/pread()
#define _DEFAULT_SOURCE    // endian.h:

#include "nbfc.h"
#include "macros.h"
#include "sleep.h"
#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "parse_number.h"
#include "parse_double.h"
#include "program_name.c"
#include "log.h"

#include <stdbool.h> // bool
#include <stdio.h>   // printf
#include <string.h>  // strchr
#include <stdlib.h>  // atext
#include <getopt.h>  // getopt_long

#include "error.c"        // src
#include "ec.c"           // src
#include "ec_linux.c"     // src
#include "ec_sys_linux.c" // src
#include "memory.c"       // src
#include "model_config.c" // src
#include "nxjson.c"       // src

static EC_VTable*   ec;
static volatile int quit;

static const struct option long_options[] = {
  {"embedded-controller",   required_argument, 0, 'e'},
  {"fan-register",          required_argument, 0, 'f'},
  {"fan-values",            required_argument, 0, 'F'},
  {"bruteforce-registers",  required_argument, 0, 'b'},
  {"bruteforce-values",     required_argument, 0, 'v'},
  {"sleep",                 required_argument, 0, 's'},
  {0,                       0,                 0,  0 }
};

static const char options_str[] = "e:f:F:b:v:s:";

static struct {
  float         sleep;
  int           fan_register;
  array_of(int) fan_values;
  array_of(int) bruteforce_registers;
  array_of(int) bruteforce_values;
} options = {
  0.5,
  -1,
  {NULL, 0},
  {NULL, 0},
  {NULL, 0}
};

static int  expand_ints(const char*, array_of(int)*);
static void reset_embedded_controller();
static void bruteforce();

int main(int argc, char* const argv[]) {
  Program_Name_Set(argv[0]);

  ec = NULL;

  const char* err;
  int o, option_index;
  while ((o = getopt_long(argc, argv, options_str, long_options, &option_index)) != -1) {
    switch (o) {
    case 'e':
      switch(EmbeddedControllerType_FromString(optarg)) {
        case EmbeddedControllerType_ECSysLinux:     ec = &EC_SysLinux_VTable;      break;
        case EmbeddedControllerType_ECSysLinuxACPI: ec = &EC_SysLinux_ACPI_VTable; break;
        case EmbeddedControllerType_ECLinux:        ec = &EC_Linux_VTable;         break;
        default:
          Log_Error("-e|--embedded-controller: Invalid value: %s\n", optarg);
          return NBFC_EXIT_CMDLINE;
      }
      break;
    case 'f':
      options.fan_register = parse_number(optarg, 0, 255, &err);
      if (err) {
        Log_Error("-f|--fan-register: %s\n", err);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    case 'F':
      if (! expand_ints(optarg, &options.fan_values))
        return NBFC_EXIT_CMDLINE;
      printf("Fan-Values: ");
      for_each_array(int*, i, options.fan_values)
        printf("%d,", *i);
      printf("\n");
      break;
    case 'b':
      if (! expand_ints(optarg, &options.bruteforce_registers))
        return NBFC_EXIT_CMDLINE;
      printf("Bruteforce-registers: ");
      for_each_array(int*, i, options.bruteforce_registers)
        printf("%d,", *i);
      printf("\n");
      break;
    case 'v':
      if (! expand_ints(optarg, &options.bruteforce_values))
        return NBFC_EXIT_CMDLINE;
      printf("Bruteforce-values: ");
      for_each_array(int*, i, options.bruteforce_values)
        printf("%d,", *i);
      printf("\n");
      break;
    case 's':
      options.sleep = parse_double(optarg, 0.1, 100, &err);
      if (err) {
        Log_Error("Invalid value for -s|--sleep: %s\n", optarg);
        return NBFC_EXIT_CMDLINE;
      }
      break;
    default:
      return NBFC_EXIT_CMDLINE;
    }
  }

  if (options.fan_register == -1) {
    Log_Error("Option -f|--fan-register not given\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (options.fan_values.size == 0) {
    Log_Error("Option -F|--fan-values not given\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (options.bruteforce_registers.size == 0) {
    Log_Error("Option -b|--bruteforce-registers not given\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (options.bruteforce_values.size == 0) {
    Log_Error("Option -v|--bruteforce-values not given\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (geteuid()) {
    Log_Error("This program must be run as root\n");
    return NBFC_EXIT_FAILURE;
  }

  if (ec == NULL) {
    Error* e = EC_FindWorking(&ec);
    e_die();
  }

  Error* e = ec->Open();
  e_die();

  atexit(reset_embedded_controller);
  bruteforce();
}

static int expand_ints(const char* s, array_of(int)* array) {
  const char* err;

  while (*s) {
    char* comma = strchr(s, ',');
    if (comma)
      *comma = '\0';

    char* dash = strchr(s, '-');
    if (dash) {
      *dash = '\0';

      const char* first = s;
      const char* second = dash + 1;

      int from = parse_number(first, 0, 255, &err);
      if (err) {
        Log_Error("Invalid number: %s: %s\n", first, err);
        return false;
      }

      int to = parse_number(second, 0, 255, &err);
      if (err) {
        Log_Error("Invalid number: %s: %s\n", second, err);
        return false;
      }

      if (from > to) {
        Log_Error("From (%d) is larger than to (%d): %s\n", from, to, s);
        return false;
      }

      for (int i = from; i <= to; ++i) {
        array->size++;
        array->data = Mem_Realloc(array->data, array->size * sizeof(int));
        array->data[array->size - 1] = i;
      }
    }
    else {
      int from_and_to = parse_number(s, 0, 255, &err);
      if (err) {
        Log_Error("Invalid number: %s: %s\n", s, err);
        return false;
      }

      array->size++;
      array->data = Mem_Realloc(array->data, array->size * sizeof(int));
      array->data[array->size - 1] = from_and_to;
    }

    if (comma)
      s = comma + 1;
    else
      s = "";
  }

  return true;
}

static struct {
  int fan_register_oldvalue;
  int brutefoce_register;
  int bruteforce_register_oldvalue;
} state = {
  -1,
  -1,
  0
};

static void reset_embedded_controller() {
  printf("Resetting embedded controller\n");

  if (state.fan_register_oldvalue >= 0)
    ec->WriteByte(options.fan_register, state.fan_register_oldvalue);
  if (state.brutefoce_register >= 0)
    ec->WriteByte(state.brutefoce_register, state.bruteforce_register_oldvalue);
}

static void bruteforce() {
  uint8_t byte;
  Error* e;

  e = ec->ReadByte(options.fan_register, &byte);
  e_die();
  state.fan_register_oldvalue = byte;

  for_each_array(int*, register_, options.bruteforce_registers) {
    state.brutefoce_register = *register_;
    e = ec->ReadByte(*register_, &byte);
    e_die();
    state.bruteforce_register_oldvalue = byte;

    for_each_array(int*, value, options.bruteforce_values) {
      e = ec->WriteByte(*register_, *value);
      e_die();

      for_each_array(int*, fan_speed_value, options.fan_values) {
        e = ec->WriteByte(options.fan_register, *fan_speed_value);
        e_die();

        printf("Register = %d (%X), Value = %d (%X), FanSpeedValue = %d (%X)\n",
          *register_, *register_, *value, *value, *fan_speed_value, *fan_speed_value);
        sleep_ms(options.sleep * 1000);
      }
    }

    e = ec->WriteByte(*register_, state.bruteforce_register_oldvalue);
    e_die();
  }
}
