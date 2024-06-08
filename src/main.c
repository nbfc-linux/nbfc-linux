#include "nbfc.h"
#include "service.h"
#include "service_config.h"
#include "server.h"
#include "error.h"
#include "log.h"
#include "ec.h"
#include "model_config.h"
#include "pidfile.h"
#include "generated/nbfc_service.help.h"
#include "quit.h"

#include <signal.h> // signal, SIGINT, SIGTERM
#include <stdio.h>  // printf
#include <stdlib.h> // exit, atexit
#include <locale.h> // setlocale, LC_NUMERIC
#include <getopt.h> // getopt_long
#include <unistd.h> // fork

EC_VTable* ec;

static void sig_handler(int sig) {
  if (sig == SIGTERM || sig == SIGINT)
    quit = true;
}

static struct option cli_options[] = {
  {"help",                no_argument,       NULL, 'h'},
  {"version",             no_argument,       NULL, 'v'},
  {"embedded-controller", required_argument, NULL, 'e'},
  {"read-only",           no_argument,       NULL, 'r'},
  {"fork",                no_argument,       NULL, 'f'},
  {"debug",               no_argument,       NULL, 'd'},
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
  Error* e;

  Program_Name_Set(argv[0]);

  setlocale(LC_NUMERIC, "C"); // for json floats

  signal(SIGINT,  sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGUSR1, sig_handler);
  signal(SIGUSR2, sig_handler);

  options.service_config = NBFC_SERVICE_CONFIG;
  options.embedded_controller_type = EmbeddedControllerType_Unset;
  parse_opts(argc, argv);

  Log_Info("Running version " NBFC_VERSION "\n");
  Log_Info("SYSCONFDIR is '%s'\n", SYSCONFDIR);
  Log_Info("DATADIR is '%s'\n", DATADIR);
  Log_Info("RUNSTATEDIR is '%s'\n", RUNSTATEDIR);

  if (options.read_only)
    Log_Info("Read-only mode enabled\n");

  e = PID_Write(true);
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  atexit(PID_Cleanup);

  // We fork early because we use threads. However, we need to confirm if the child
  // process has been initialized correctly. To achieve this, we communicate via this pipe.
  // If there is any data (read() succeeds), it means the service was initialized correctly.
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    Log_Error("pipe(): %s\n", strerror(errno));
    return NBFC_EXIT_FAILURE;
  }

  if (options.fork) {
    int pid = fork();
    switch (pid) {
    case -1:
      Log_Error("fork(): %s\n", strerror(errno));
      return NBFC_EXIT_FAILURE;
    case 0:
      close(pipefd[0]);
      break;
    default:
      close(pipefd[1]);

      char buf;
      if (read(pipefd[0], &buf, 1) == 1)
        _exit(NBFC_EXIT_SUCCESS);
      else
        _exit(NBFC_EXIT_INIT);
    }
  }

  // We got a new PID
  e = PID_Write(false);
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  atexit(Service_Cleanup);
  atexit(Server_Close);

  e = Service_Init();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  e = Server_Init();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  e = Server_Start();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  if (write(pipefd[1], "1", 1) == -1) {
    Log_Error("write(): %s\n", strerror(errno));
    return NBFC_EXIT_FAILURE;
  }
  close(pipefd[1]); 

  while (!quit) {
    Service_Loop();
  }

  Server_Stop();

  return 0;
}

