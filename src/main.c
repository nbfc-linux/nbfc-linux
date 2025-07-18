#include "nbfc.h"
#include "service.h"
#include "service_config.h"
#include "server.h"
#include "error.h"
#include "file_utils.h"
#include "log.h"
#include "ec.h"
#include "model_config.h"
#include "pidfile.h"
#include "program_name.h"
#include "nvidia.h"
#include "help/nbfc_service.help.h"
#include "sleep.h"
#include "mkdir_p.h"

#include <errno.h>  // errno
#include <string.h> // strerror
#include <signal.h> // signal, SIGINT, SIGTERM
#include <stdio.h>  // printf
#include <stdlib.h> // exit, atexit, realpath
#include <locale.h> // setlocale, LC_NUMERIC
#include <getopt.h> // getopt_long
#include <unistd.h> // fork, setsid, chdir, geteuid
#include <sys/time.h> // gettimeofday

EC_VTable* ec;

static volatile bool quit = false;

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
    case 'd':
      options.debug = 1;
      Log_LogLevel = LogLevel_Debug;
      break;
    case 'c':
      if (! realpath(optarg, options.service_config)) {
        Log_Error("%s: %s: %s\n", "-c|--config-file", optarg, strerror(errno));
        exit(NBFC_EXIT_CMDLINE);
      }
      break;
    case 'v':  printf("nbfc-linux " NBFC_VERSION "\n"); exit(0);   break;
    case 'h':  printf(NBFC_SERVICE_HELP_TEXT, argv[0]); exit(0);   break;
    case 'r':  options.read_only      = 1;                         break;
    case 'f':  options.fork           = 1;                         break;
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

  options.embedded_controller_type = EmbeddedControllerType_Unset;
  snprintf(options.service_config, sizeof(options.service_config), "%s", NBFC_SERVICE_CONFIG);
  parse_opts(argc, argv);

  // Change working directory to root to ensure that the process doesn't block
  // the unmounting of filesystems by holding a directory open.
  chdir("/");

  if (geteuid() != 0) {
    Log_Error("This program must be run as root\n");
    exit(NBFC_EXIT_FAILURE);
  }

  mkdir_p(NBFC_CONFIG_DIR, 0755);
  mkdir_p(NBFC_MODEL_CONFIGS_DIR_MUTABLE, 0755);

  Log_Init(options.fork);
  atexit(Log_Close);

  Log_Info("NBFC-Linux comes with no warranty. Run `nbfc warranty` for details.\n");
  Log_Info("Running version " NBFC_VERSION "\n");
  Log_Info("SYSCONFDIR is '%s'\n", SYSCONFDIR);
  Log_Info("DATADIR is '%s'\n", DATADIR);
  Log_Info("RUNSTATEDIR is '%s'\n", RUNSTATEDIR);
  Log_Info("Available Embedded Controllers: "
#if ENABLE_EC_SYS
    "ec_sys "
#endif
#if ENABLE_EC_ACPI
    "acpi_ec "
#endif
#if ENABLE_EC_DEV_PORT
    "dev_port "
#endif
#if ENABLE_EC_DUMMY
    "dummy "
#endif
    "\n"
  );

  // Sets the OOM (Out-Of-Memory) score adjustment for this process to -1000,
  // which tells the Linux kernel to never kill this process, even under
  // extreme memory pressure.
  if (write_file("/proc/self/oom_score_adj", O_WRONLY, 0, "-1000\n", 6) < 0) {
    Log_Error("%s: %s\n", "/proc/self/oom_score_adj", strerror(errno));
    exit(NBFC_EXIT_FAILURE);
  }

  if (options.read_only)
    Log_Info("Read-only mode enabled\n");

  e = PID_Write(true);
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  atexit(PID_Cleanup);

  e = Service_Init();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  atexit(Service_Cleanup);

  e = Server_Init();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_INIT;
  }

  atexit(Server_Close);

  // We need to call Service_Loop once to catch and print errors early,
  // since we will fork and close STDERR later.
  e = Service_Loop();
  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (options.fork) {
    Nvidia_Close();

    switch (fork()) {
    case -1:
      Log_Error("fork(): %s\n", strerror(errno));
      return NBFC_EXIT_FAILURE;
    case 0:
      // Create a new session and detach from the controlling terminal
      // to run the process as a true daemon.
      if (setsid() < 0) {
        Log_Error("setsid(): %s\n", strerror(errno));
        return NBFC_EXIT_FAILURE;
      }

      // We got a new PID
      e = PID_Write(false);
      if (e) {
        Log_Error("%s\n", err_print_all(e));
        return NBFC_EXIT_INIT;
      }

      // Close standard file descriptors (stdin, stdout, stderr)
      // because a daemon should not be attached to a terminal.
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

      // Re-initialize nvidia after fork
      Nvidia_Init();

      break;
    default:
      _exit(NBFC_EXIT_SUCCESS);
    }
  }

  int failures = 0;

  while (!quit) {
    // ========================================================================
    // Run the service loop.
    // This does the main work of the service.
    // ========================================================================
    e = Service_Loop();
    if (! e) {
      failures = 0;
    }
    else {
      if (++failures >= 100) {
        Log_Error("%s\n", err_print_all(e));
        Log_Error("We tried %d times, exiting now...\n", failures);
        return NBFC_EXIT_FAILURE;
      }
      sleep_ms(10);
      continue;
    }

    // ========================================================================
    // Run the server loop for Service_Model_Config.EcPollInterval miliseconds.
    // ========================================================================
    struct timeval start, current;
    gettimeofday(&start, NULL);

    while (!quit) {
      gettimeofday(&current, NULL);
      int elapsed = (current.tv_sec - start.tv_sec) * 1000 + (current.tv_usec - start.tv_usec) / 1000;
      int timeout = Service_Model_Config.EcPollInterval - elapsed;

      if (timeout <= 0)
        break;

      e = Server_Loop(timeout);
      e_warn();
    }
  }

  return 0;
}
