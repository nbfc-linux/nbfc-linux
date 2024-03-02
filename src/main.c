#include "nbfc.h"
#include "service.h"
#include "error.h"
#include "log.h"
#include "ec.h"
#include "model_config.h"
#include "generated/nbfc_service.help.h"

#include <signal.h> // signal, SIGINT, SIGTERM
#include <stdio.h>  // printf
#include <stdlib.h> // exit, atexit
#include <locale.h> // setlocale, LC_NUMERIC
#include <getopt.h> // getopt_long

EC_VTable* ec;

static volatile int quit;
static void sig_handler(int i) { quit = i; }

static struct option cli_options[] = {
  {"help",                no_argument,       NULL, 'h'},
  {"version",             no_argument,       NULL, 'v'},
  {"embedded-controller", required_argument, NULL, 'e'},
  {"read-only",           no_argument,       NULL, 'r'},
  {"fork",                no_argument,       NULL, 'f'},
  {"debug",               no_argument,       NULL, 'd'},
  {"state-file",          required_argument, NULL, 's'},
  {"config-file",         required_argument, NULL, 'c'},
  {0,                     0,                 0,     0 },
};

static const char cli_options_str[] = "hve:rfds:c:";

static void parse_opts(int argc, char* const argv[]) {
  int o;
  int option_index;
  while ((o = getopt_long(argc, argv, cli_options_str, cli_options, &option_index)) != -1) {
    switch (o) {
    case 'e':
      options.embedded_controller_type = EmbeddedControllerType_FromString(optarg);
      if (options.embedded_controller_type == EmbeddedControllerType_Unset) {
        Log_Error("Invalid value for %s: %s\n", "-e|--embedded-controller", optarg);
        exit(NBFC_EXIT_CMDLINE);
      }
      break;
    case 'v':  printf("nbfc-linux " NBFC_VERSION "\n"); exit(0);   break;
    case 'h':  printf(NBFC_SERVICE_HELP_TEXT, argv[0]); exit(0);   break;
    case 'r':  options.read_only      = 1;                         break;
    case 'f':  options.fork           = 1;                         break;
    case 'd':  options.debug          = 1;                         break;
    case 's':  options.state_file     = optarg;                    break;
    case 'c':  options.service_config = optarg;                    break;
    default:   exit(NBFC_EXIT_CMDLINE);
    }
  }

  if (optind < argc) {
    Log_Error("Too much arguments\n");
    exit(NBFC_EXIT_CMDLINE);
  }
}

int main(int argc, char* const argv[])
{
  Program_Name_Set(argv[0]);

  setlocale(LC_NUMERIC, "C"); // for json floats

  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);

  options.service_config = NBFC_SERVICE_CONFIG;
  options.state_file     = NBFC_STATE_FILE;
  options.embedded_controller_type = EmbeddedControllerType_Unset;
  parse_opts(argc, argv);

  Log_Info("Running version " NBFC_VERSION "\n");
  Log_Info("SYSCONFDIR is '%s'\n", SYSCONFDIR);
  Log_Info("DATADIR is '%s'\n", DATADIR);
  Log_Info("RUNSTATEDIR is '%s'\n", RUNSTATEDIR);

  if (options.read_only)
    Log_Info("Read-only mode enabled\n");

  atexit(Service_Cleanup);

  Error* e = Service_Init();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    exit(NBFC_EXIT_INIT);
  }

  while (!quit) {
    Service_Loop();
  }

  return 0;
}

