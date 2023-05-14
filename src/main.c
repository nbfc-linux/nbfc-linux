#include "nbfc.h"
#include "service.h"

#include "ec_linux.h"
#include "ec_sys_linux.h"
#include "ec_dummy.h"
#include "ec_debug.h"

#include "error.h"
#include "memory.h"
#include "model_config.h"
#include "optparse/optparse.h"
#include "generated/nbfc_service.help.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>

EC_VTable* ec;

static volatile int quit;
static void sig_handler(int i) { quit = i; }

static cli99_option cli_options[] = {
#define G1 cli99_exclusive_group(1) // readonly <or> fork
#define G2 cli99_exclusive_group(2) // debug    <or> fork
  {"-h|--help",                'h',  0},
  {"-v|--version",             'v',  0},
  {"-e|--embedded-controller", 'e',  1},
  {"-r|--readonly",            'r',  0},
  {"-f|--fork",                'f',  0},
  {"-d|--debug",               'd',  0},
  {"-s|--state-file",          's',  1},
  {"-c|--config-file",         'c',  1},
  {"--critical-temperature",   '!',  1|cli99_type(float)},
  {"--check-model-config",     'C',  1},
  cli99_options_end()
#undef G1
#undef G2
};

static void check_model_config(const char* file) {
  char path[PATH_MAX];

  if (strchr(file, '/')) {
    snprintf(path, PATH_MAX, "%s", file);
  }
  else {
    snprintf(path, PATH_MAX, "%s/%s.json", NBFC_CONFIGS_DIR, file);
  }

  Error* e = ModelConfig_FromFile(&model_config, path);
  e_die();

  e = ModelConfig_Validate(&model_config);
  e_die();
}

static void parse_opts(int argc, char* const argv[]) {
  cli99 p;
  cli99_Init(&p, argc, argv, cli_options, cli99_options_python);

  int o;
  while ((o = cli99_GetOpt(&p))) {
    switch (o) {
    case 'e':
      /**/ if (! strcmp(p.optarg, "dummy"))        options.embedded_controller_type = EmbeddedControllerType_ECDummy;
      else if (! strcmp(p.optarg, "ec_linux"))     options.embedded_controller_type = EmbeddedControllerType_ECLinux;
      else if (! strcmp(p.optarg, "ec_sys_linux")) options.embedded_controller_type = EmbeddedControllerType_ECSysLinux;
      else if (! strcmp(p.optarg, "ec_acpi"))      options.embedded_controller_type = EmbeddedControllerType_ECSysLinuxACPI;
      else {
        fprintf(stderr, "Invalid value for %s: %s\n", p.optopt, p.optarg);
        exit(NBFC_EXIT_CMDLINE);
      }
      break;
    case 'v':  printf("nbfc-linux " NBFC_VERSION "\n"); exit(0);   break;
    case 'h':  printf(NBFC_SERVICE_HELP_TEXT, argv[0]); exit(0);   break;
    case 'r':  options.read_only      = 1;                         break;
    case 'f':  options.fork           = 1;                         break;
    case 'd':  options.debug          = 1;                         break;
    case 's':  options.state_file     = p.optarg;                  break;
    case 'c':  options.service_config = p.optarg;                  break;
    case '!':  options.critical_temperature = p.optval.d;          break;
    case 'C':  check_model_config(p.optarg); exit(0);              break;
    default:
      cli99_ExplainError(&p);
      exit(NBFC_EXIT_CMDLINE);
    }
  }

  if (!cli99_End(&p)) {
    fprintf(stderr, "Too much arguments\n");
    exit(NBFC_EXIT_CMDLINE);
  }
}

int main(int argc, char* const argv[])
{
  setlocale(LC_NUMERIC, "C"); // for json floats
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  options.service_config = NBFC_SERVICE_CONFIG;
  options.state_file     = NBFC_STATE_FILE;
  options.embedded_controller_type = EmbeddedControllerType_Unset;
  parse_opts(argc, argv);

  if (options.read_only)
    fprintf(stderr, "readonly mode enabled\n");

  atexit(Service_Cleanup);

  Error* e = Service_Init();
  if (e) {
    err_print_all(e);
    exit(NBFC_EXIT_INIT);
  }

  while (!quit) {
    Service_HandleError(Service_Loop());
  }

  return 0;
}

