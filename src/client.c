#define _XOPEN_SOURCE 500 // unistd.h: pwrite()/pread(), string.h: strdup
#define _DEFAULT_SOURCE   // endian.h: htole16(), le16toh()

#define NX_JSON_CALLOC(SIZE) ((nx_json*) Mem_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (Mem_Free((void*) (JSON)))

#include <ctype.h>     // tolower
#include <dirent.h>    // DIR, opendir, readdir, closedir
#include <errno.h>     // ENODATA
#include <fcntl.h>     // O_WRONLY, O_CREAT, O_TRUNC
#include <float.h>     // FLT_MAX
#include <locale.h>    // setlocale, LC_NUMERIC
#include <limits.h>    // INT_MAX, PATH_MAX
#include <signal.h>    // kill, SIGINT
#include <stdbool.h>   // bool
#include <stdio.h>     // printf, snprintf
#include <stdlib.h>    // exit, realpath, system, WEXITSTATUS, qsort
#include <string.h>    // strcmp, strcat, strcspn, strrchr, strerror
#include <sys/stat.h>  // S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP, S_IROTH
#include <unistd.h>    // access, F_OK, geteuid, unlink
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "generated/client.help.h"
#include "log.h"
#include "macros.h"
#include "nbfc.h"
#include "nxjson_utils.h"
#include "parse_number.h"
#include "parse_double.h"
#include "protocol.h"
#include "sleep.h"
#include "slurp_file.h"
#include "stringbuf.h"
#include "service_config.h"

#include "error.c"
#include "optparse/optparse.c"
#include "model_config.c"
#include "memory.c"
#include "program_name.c"
#include "protocol.c"
#include "nxjson.c"
#include "reverse_nxjson.c"
#include "service_config.c"

#define DmiIdDirectoryPath "/sys/devices/virtual/dmi/id"
#define RecommendedConfigMatchThreshold 0.7f

static ServiceInfo service_info;

static const cli99_option main_options[] = {
  {"-v|--version",  -'v', 0},
  {"-h|--help",     -'h', 0},
  {"--python-hack", -'p', 0},
  {"command",        'C', 1 | cli99_required_option},
  cli99_options_end()
};

static const cli99_option status_options[] = {
  cli99_include_options(&main_options),
  {"-a|--all",      -'a', 0},
  {"-s|--service",  -'s', 0},
  {"-f|--fan",      -'f', 1},
  {"-w|--watch",    -'w', 1},
  cli99_options_end()
};

static const cli99_option config_options[] = {
  cli99_include_options(&main_options),
  {"-l|--list",       -'l', 0},
  {"-r|--recommend",  -'r', 0},
  {"-s|--set",        -'s', 1},
  {"-a|--apply",      -'a', 1},
  cli99_options_end()
};

static const cli99_option set_options[] = {
  cli99_include_options(&main_options),
  {"-a|--auto",       -'a', 0},
  {"-s|--speed",      -'s', 1},
  {"-f|--fan",        -'f', 1},
  cli99_options_end()
};

static const cli99_option start_options[] = {
  cli99_include_options(&main_options),
  {"-r|--read-only",  -'r', 0},
  cli99_options_end()
};

static const cli99_option show_variable_options[] = {
  cli99_include_options(&main_options),
  {"variable",         'V', 1},
  cli99_options_end()
};

#define NBFC_CLIENT_COMMANDS \
  o("start",          Start,          START,           start)         \
  o("stop",           Stop,           STOP,            main)          \
  o("restart",        Restart,        RESTART,         start)         \
  o("status",         Status,         STATUS,          status)        \
  o("config",         Config,         CONFIG,          config)        \
  o("set",            Set,            SET,             set)           \
  o("wait-for-hwmon", Wait_For_Hwmon, WAIT_FOR_HWMON,  main)          \
  o("get-model-name", Get_Model_Name, GET_MODEL,       main)          \
  o("complete-fans",  Complete_Fans,  COMPLETE_FANS,   main)          \
  o("show-variable",  Show_Variable,  SHOW_VARIABLE,   show_variable) \
  o("help",           Help,           HELP,            main)

enum Command {
#define o(COMMAND, ENUM, HELP, OPTIONS)  Command_ ## ENUM,
  NBFC_CLIENT_COMMANDS
#undef o
};

static const char *HelpTexts[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  CLIENT_ ## HELP ## _HELP_TEXT,
  NBFC_CLIENT_COMMANDS
#undef o
};

static const cli99_option *Options[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  OPTIONS ## _options,
  NBFC_CLIENT_COMMANDS
#undef o
};

static enum Command Command_From_String(const char* s) {
  const char* commands[] = {
#define o(COMMAND, ENUM, HELP, OPTIONS)  COMMAND,
    NBFC_CLIENT_COMMANDS
#undef o
  };

  for (int i = 0; i < ARRAY_SSIZE(commands); ++i)
    if (!strcmp(commands[i], s))
      return (enum Command) i;

  return (enum Command) -1;
}

static struct {
  array_of(int) fans;
  array_of(float) speeds;
  const char* config;
  const char* variable;
  bool python_hack;
  bool a;      // all/auto/apply
  bool l;      // list
  bool r;      // recommend/read-only
  bool s;      // set
  float watch; // watch time
} options = {0};

static int Service_Start(bool);
static int Service_Stop();
static int Service_Restart(bool);
static int Config();
static int Set();
static int Status();
static int Wait_For_Hwmon();
static int Get_Model_Name();
static int Show_Variable();
static int Complete_Fans();

int main(int argc, char *const argv[]) {
  if (argc == 1) {
    printf(CLIENT_HELP_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }

  Program_Name_Set(argv[0]);
  setlocale(LC_NUMERIC, "C"); // for json floats
  mkdir(NBFC_CONFIG_DIR, 0755);

  int o;
  const char* err;
  enum Command cmd;
  cli99 p;
  cli99_Init(&p, argc, argv, main_options, cli99_options_python);
  while ((o = cli99_GetOpt(&p))) {
    switch (o) {
    case 'C':
      cmd = Command_From_String(p.optarg);
      if (cmd == (enum Command) -1) {
        Log_Error("Invalid command: %s\n", p.optarg);
        return NBFC_EXIT_CMDLINE;
      }

      if (cmd == Command_Help) {
        printf("%s", HelpTexts[Command_Help]);
        return NBFC_EXIT_SUCCESS;
      }
      cli99_SetOptions(&p, Options[cmd], false);
      break;
    case 'V':
      options.variable = p.optarg;
      break;
    case -'h':
      printf("%s", HelpTexts[cmd]);
      return NBFC_EXIT_SUCCESS;
    case -'v':
      printf("nbfc " NBFC_VERSION "\n");
      return NBFC_EXIT_SUCCESS;
    case -'p':
      options.python_hack = 1;
      break;
    case -'a':
      options.a = true;
      if (cmd == Command_Config) {
        if (options.s) {
          Log_Error("You cannot use --apply and --set at the same time\n");
          return NBFC_EXIT_CMDLINE;
        }

        options.config = p.optarg;
      }
      break;
    case -'l':
      options.l = true;
      break;
    case -'r':
      options.r = true;
      break;
    case -'w':
      options.watch = parse_double(p.optarg, 0.1, FLT_MAX, &err);
      if (err) {
        Log_Error("-w|--watch: %s\n", err);
        return NBFC_EXIT_FAILURE;
      }
      break;
    case -'f':
      if (cmd == Command_Set || cmd == Command_Status) {
        const int fan = parse_number(p.optarg, 0, INT_MAX, &err);
        if (err) {
          Log_Error("-f|--fan: %s\n", err);
          return NBFC_EXIT_FAILURE;
        }

        options.fans.size++;
        options.fans.data = Mem_Realloc(options.fans.data, options.fans.size * sizeof(int));
        options.fans.data[options.fans.size - 1] = fan;
      }
      break;
    case -'s':
      if (cmd == Command_Set) {
        const float speed = parse_double(p.optarg, 0, 100, &err);
        if (err) {
          Log_Error("-s|--speed: %s\n", err);
          return NBFC_EXIT_FAILURE;
        }

        options.speeds.size++;
        options.speeds.data = Mem_Realloc(options.speeds.data, options.speeds.size * sizeof(float));
        options.speeds.data[options.speeds.size - 1] = speed;
      } else {
        options.s = true;
        if (cmd == Command_Config && options.a) {
          Log_Error("You cannot use --apply and --set at the same time\n");
          return NBFC_EXIT_FAILURE;
        }
        options.config = p.optarg;
      }
      break;
    default:
      cli99_ExplainError(&p);
      return NBFC_EXIT_CMDLINE;
    }
  }

  switch (cmd) {
  case Command_Start:          return Service_Start(options.r);
  case Command_Stop:           return Service_Stop();
  case Command_Restart:        return Service_Restart(options.r);
  case Command_Config:         return Config();
  case Command_Set:            return Set();
  case Command_Status:         return Status();
  case Command_Wait_For_Hwmon: return Wait_For_Hwmon();
  case Command_Get_Model_Name: return Get_Model_Name();
  case Command_Show_Variable:  return Show_Variable();
  case Command_Complete_Fans:  return Complete_Fans();
  default:                     return NBFC_EXIT_FAILURE;
  }
}

static char* to_lower(const char*);
static bool  str_starts_with_ignorecase(const char*, const char*);
static float word_difference(const char*, const char*);

Error* Client_Communicate(const nx_json* in, char** buf, const nx_json** out) {
  int sock;
  struct sockaddr_un serv_addr;
  Error* e = NULL;

  sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0)
    return err_stdlib(0, "socket()");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sun_family = AF_UNIX;
  snprintf(serv_addr.sun_path, sizeof(serv_addr.sun_path), NBFC_SOCKET_PATH);

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    e = err_stdlib(0, "connect()");
    goto error;
  }

  e = Protocol_Send_Json(sock, in);
  if (e)
    goto error;

  e = Protocol_Receive_Json(sock, buf, out);
  if (e)
    goto error;

error:
  close(sock);
  return e;
}

static const char* get_system_product() {
  static char buf[128];

  if (slurp_file(buf, sizeof(buf), DmiIdDirectoryPath "/product_name") == -1)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get product name. Failed to read " DmiIdDirectoryPath "/product_name: %s\n", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

static const char* get_system_vendor() {
  static char buf[128];

  if (slurp_file(buf, sizeof(buf), DmiIdDirectoryPath "/sys_vendor") == -1)
    goto error;

  buf[strcspn(buf, "\n")] = '\0';

  if (!*buf) {
    errno = ENODATA;
    goto error;
  }

  return buf;

error:
  Log_Error("Could not get system vendor. Failed to read " DmiIdDirectoryPath "/sys_vendor: %s\n", strerror(errno));
  exit(NBFC_EXIT_FAILURE);
}

static const char* get_model_name() {
  static char model_name[256];

  struct vendor_alias { const char* key; const char* value; };

  const struct vendor_alias vendor_aliases[] = {
    { "Hewlett-Packard", "HP" },
    { NULL, NULL }
  };

  const char* product = get_system_product();
  const char* vendor = get_system_vendor();

  for (const struct vendor_alias* alias = vendor_aliases; alias->key; ++alias) {
    if (! strcmp(vendor, alias->key)) {
      vendor = alias->value;
      break;
    }
  }

  if (str_starts_with_ignorecase(product, vendor))
    snprintf(model_name, sizeof(model_name), "%s", product);
  else
    snprintf(model_name, sizeof(model_name), "%s %s", vendor, product);

  return model_name;
}

static void check_root() {
  if (geteuid()) {
    Log_Error("This operation must be run as root\n");
    exit(NBFC_EXIT_FAILURE);
  }
}

static int get_service_pid() {
  const char* err;
  char buf[32];
  if (slurp_file(buf, sizeof(buf), NBFC_PID_FILE) == -1) {
    if (errno == ENOENT)
      return -1;
    else {
      err = strerror(errno);
      goto error;
    }
  }

  // trim the newline
  buf[strcspn(buf, "\n")] = '\0';

  int pid = parse_number(buf, 0, INT_MAX, &err);
  if (err) {
error:
    Log_Error("Failed to read the pid file: " NBFC_PID_FILE ": %s\n", err);
    exit(NBFC_EXIT_FAILURE);
  }

  return pid;
}

static int Service_Start(bool read_only) {
  check_root();
  int pid = get_service_pid();
  if (pid != -1) {
    Log_Info("Service already running (pid: %d)\n", pid);
    return NBFC_EXIT_SUCCESS;
  }

  char cmd[64] = "nbfc_service -f";
  if (options.python_hack)
    strcat(cmd, " --python-hack");
  if (read_only)
    strcat(cmd, " -r");
  int ret = system(cmd);
  if (ret == -1) {
    Log_Error("Failed to start process: %s\n", strerror(errno));
    return NBFC_EXIT_FAILURE;
  }
  if (WEXITSTATUS(ret) == 127) {
    Log_Error("Can't run nbfc_service, make sure the binary is installed\n");
    return NBFC_EXIT_FAILURE;
  }
  return WEXITSTATUS(ret);
}

static int Service_Stop() {
  check_root();
  int pid = get_service_pid();
  if (pid == -1) {
    Log_Error("Service not running\n");
    return NBFC_EXIT_SUCCESS;
  }
  Log_Info("Killing nbfc_service (%d)\n", pid);
  if (kill(pid, SIGINT) == -1) {
    Log_Error("Failed to kill nbfc_service process (%d): %s\n", pid, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }
  unlink(NBFC_PID_FILE);
  return NBFC_EXIT_SUCCESS;
}

static int Service_Restart(bool read_only) {
  check_root();
  Service_Stop();
  sleep(1);
  return Service_Start(read_only);
}

struct ConfigFile {
  char *config_name;
  float diff;
};
typedef struct ConfigFile ConfigFile;
declare_array_of(ConfigFile);

// get an array of config names
static array_of(ConfigFile) get_configs() {
  size_t capacity = 512;
  array_of(ConfigFile) files = {
    .data = Mem_Malloc(capacity * sizeof(struct ConfigFile)),
    .size = 0
  };

  DIR* directory = opendir(NBFC_MODEL_CONFIGS_DIR);
  if (!directory) {
    Log_Error("Failed to open directory `" NBFC_MODEL_CONFIGS_DIR "': %s\n", strerror(errno));
    exit(NBFC_EXIT_FAILURE);
  }

  struct dirent* file;
  while ((file = readdir(directory)) != NULL) {
    if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
      continue;

    // remove .json extension
    char* dot = strrchr(file->d_name, '.');
    if (dot)
      *dot = '\0';

    if (files.size == capacity) {
      capacity *= 2;
      files.data = Mem_Realloc(files.data, capacity * sizeof(struct ConfigFile));
    }

    files.data[files.size++].config_name = Mem_Strdup(file->d_name);
  }

  closedir(directory);
  return files;
}

// compare function for qsort
int compare_config_by_name(const void *a, const void *b) {
  return strcmp(((struct ConfigFile *)a)->config_name, ((struct ConfigFile *)b)->config_name);
}

// compare function for qsort
int compare_config_by_diff(const void *a, const void *b) {
  return (((struct ConfigFile *)b)->diff > ((struct ConfigFile *)a)->diff)
       - (((struct ConfigFile *)b)->diff < ((struct ConfigFile *)a)->diff);
}

// recommend configs in a sorted array
static array_of(ConfigFile) recommended_configs() {
  const char *product = get_model_name();
  array_of(ConfigFile) files = get_configs();
  for_each_array(ConfigFile*, file, files) {
    file->diff = word_difference(product, file->config_name);
  }
  qsort(files.data, files.size, sizeof(struct ConfigFile), compare_config_by_diff);
  return files;
}

static void ServiceConfig_Load() {
  if (access(NBFC_SERVICE_CONFIG, F_OK) != 0) {
    service_config = ServiceConfig_Unset; // Clear values
    return;
  }

  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;
  Error* e = nx_json_parse_file(&js, buf, sizeof(buf), NBFC_SERVICE_CONFIG);
  if (e)
    goto error;

  e = ServiceConfig_FromJson(&service_config, js);
  nx_json_free(js);

  if (e) {
error:
    e = err_string(e, NBFC_SERVICE_CONFIG);
    e_die();
  }
}

static Error* ServiceInfo_TryLoad() {
  Error* e;
  nx_json root = {0};
  nx_json* in = create_json_object(NULL, &root);
  create_json_string("Command", in, "status");

  char* buf = NULL;
  const nx_json* out = NULL;
  e = Client_Communicate(in, &buf, &out);
  if (e)
    goto error;

  if (out->type != NX_JSON_OBJECT) {
    e = err_string(0, "Not a JSON object");
    goto error;
  }

  const nx_json* err = nx_json_get(out, "Error");
  if (err) {
    if (err->type != NX_JSON_STRING) {
      e = err_string(0, "'Error' is not a string");
      goto error;
    }

    e = err_string(0, err->val.text);
    goto error;
  }

  e = ServiceInfo_FromJson(&service_info, out);
  if (e)
    goto error;

  e = ServiceInfo_ValidateFields(&service_info);
  if (e)
    goto error;

  for_each_array(FanInfo*, f, service_info.Fans) {
    e = FanInfo_ValidateFields(f);
    if (e)
      goto error;
  }

error:
  nx_json_free(in);
  nx_json_free(out);
  Mem_Free(buf);

  return e;
}

static inline void ServiceInfo_Load() {
  Error* e = ServiceInfo_TryLoad();
  e_die();
}

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

static int Show_Variable() {
  if (! options.variable) {
    Log_Error("Missing argument: VARIABLE\n");
    return NBFC_EXIT_CMDLINE;
  }

  int ret = NBFC_EXIT_SUCCESS;
  char* variable = to_lower(options.variable);

  if (! strcmp(variable, "config_file"))
    printf("%s\n", NBFC_SERVICE_CONFIG);
  else if (! strcmp(variable, "socket_file"))
    printf("%s\n", NBFC_SOCKET_PATH);
  else if (! strcmp(variable, "pid_file"))
    printf("%s\n", NBFC_PID_FILE);
  else if (! strcmp(variable, "model_configs_dir"))
    printf("%s\n", NBFC_MODEL_CONFIGS_DIR);
  else {
    ret = NBFC_EXIT_FAILURE;
    Log_Error("Unknown variable '%s'. Choose from 'config_file', 'socket_file', 'pid_file', 'model_configs_dir'\n", options.variable);
  }

  Mem_Free(variable);
  return ret;
}

static const char* bool_to_str(bool val) {
  static const char strings[2][6] = {"false", "true"};
  return strings[val];
}

static void print_fan_status(const FanInfo* fan) {
  printf("Fan Display Name         : %s\n"
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

static void print_service_status() {
  printf("Read-only                : %s\n"
         "Selected Config Name     : %s\n",
         bool_to_str(service_info.ReadOnly),
         service_info.SelectedConfigId);
}

static int Complete_Fans() {
  Error* e = ServiceInfo_TryLoad();
  if (e)
    return NBFC_EXIT_FAILURE;

  int idx = 0;
  for_each_array(const FanInfo*, f, service_info.Fans)
    printf("%d\t%s\n", idx++, f->Name);

  return NBFC_EXIT_SUCCESS;
}

static int Status() {
  if (!options.s && !options.a && !options.fans.size)
    options.a = true;

  while (true) {
    ServiceInfo_Load();

    if (options.a || options.s)
      print_service_status();

    if (options.a) {
      for_each_array(const FanInfo*, f, service_info.Fans) {
        printf("\n");
        print_fan_status(f);
      }
    }
    else if (options.fans.size) {
      const int fan_count = service_info.Fans.size;
      bool *vis = Mem_Calloc(sizeof(bool), fan_count);
      for_each_array(int*, fan_index, options.fans) {
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

    if (!options.watch)
      break;
    sleep_ms(options.watch * 1000);
    printf("%s", "\033c");
  }

  return NBFC_EXIT_SUCCESS;
}

static int Config() {
  if (options.l) {
    array_of(ConfigFile) files = get_configs();
    qsort(files.data, files.size, sizeof(struct ConfigFile), compare_config_by_name);
    for_each_array(ConfigFile*, file, files) {
      printf("%s\n", file->config_name);
    }
  }
  else if (options.r) {
    fprintf(stderr,
      "Note: The command 'nbfc config -r' outputs recommended configurations\n"
      "based solely on comparing your model name with configuration file names.\n"
      "This recommendation does not imply any further significance or validation\n"
      "of the configurations beyond the string matching.\n\n");

    array_of(ConfigFile) files = recommended_configs();

    if (files.size && !strcmp(files.data[0].config_name, get_model_name())) {
      printf("%s\n", files.data[0].config_name);
    }
    else {
      bool have_match = 0;
      for_each_array(ConfigFile*, file, files) {
        if (file->diff >= RecommendedConfigMatchThreshold) {
          have_match = 1;
          printf("%s\n", file->config_name);
        }
      }
      if (! have_match) {
        Log_Error("No recommended configuration files found\n");
      }
    }
  }
  else if (options.s || options.a) {
    check_root();
    char *model;
    char path[PATH_MAX];

    if (!strcmp(options.config, "auto")) {
      array_of(ConfigFile) files = recommended_configs();
      if (files.size && !strcmp(files.data[0].config_name, get_model_name())) {
        model = files.data[0].config_name;
      } else {
        Log_Error("No recommended config found to apply automatically\n");
        return NBFC_EXIT_FAILURE;
      }
    }
    else {
      if (strrchr(options.config, '/')) {
        if (! realpath(options.config, path)) {
          Log_Error("Failed to resolve path '%s': %s\n", options.config, strerror(errno));
          return NBFC_EXIT_FAILURE;
        }

        char* slash = strrchr(path, '/');
        *slash = '\0';
        model = slash + 1;

        if (strcmp(path, NBFC_MODEL_CONFIGS_DIR)) {
          Log_Error("File does not reside in model configs dir (%s): %s\n",
            NBFC_MODEL_CONFIGS_DIR, options.config);
          return NBFC_EXIT_FAILURE;
        }
      }
      else {
        snprintf(path, sizeof(path), "%s", options.config);
        model = path;
      }

      char* dot = strrchr(model, '.');
      if (dot)
        *dot = '\0';

      char file[PATH_MAX];
      snprintf(file, sizeof(file), NBFC_MODEL_CONFIGS_DIR "/%s.json", model);
      if (access(file, F_OK)) {
        Log_Error("No such config file found: %s\n", file);
        return NBFC_EXIT_FAILURE;
      }
    }

    ServiceConfig_Load();
    service_config.SelectedConfigId = model;
    Error* e = ServiceConfig_Write(NBFC_SERVICE_CONFIG);
    if (e) {
      Log_Error("%s\n", err_print_all(e));
      return NBFC_EXIT_FAILURE;
    }

    if (options.a)
      return Service_Restart(false);
  } else {
    printf(CLIENT_CONFIG_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }

  return NBFC_EXIT_SUCCESS;
}

static int Set() {
  if (!options.a && !options.speeds.size) {
    printf(CLIENT_SET_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }

  if (options.speeds.size > 1) {
    Log_Error("Multiple -s|--speed not supported\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (options.fans.size > 1) {
    Log_Error("Multiple -f|--fan not supported\n");
    return NBFC_EXIT_CMDLINE;
  }

  if (get_service_pid() == -1) {
    Log_Error("Service not running\n");
    return NBFC_EXIT_FAILURE;
  }

  nx_json root = {0};
  nx_json* in = create_json_object(NULL, &root);
  create_json_string("Command", in, "set-fan-speed");

  if (options.fans.size)
    create_json_integer("Fan", in, options.fans.data[0]);

  if (options.a)
    create_json_string("Speed", in, "auto");
  else
    create_json_double("Speed", in, options.speeds.data[0]);

  char* buf = NULL;
  const nx_json* out = NULL;
  Error* e = Client_Communicate(in, &buf, &out);
  if (e)
    goto error;

  if (out->type != NX_JSON_OBJECT) {
    e = err_string(0, "Not a JSON object");
    goto error;
  }

  const nx_json* err = nx_json_get(out, "Error");
  if (err) {
    if (err->type != NX_JSON_STRING) {
      e = err_string(0, "'Error' is not a string");
      goto error;
    }

    Log_Error("Service returned: %s\n", err->val.text);
    return NBFC_EXIT_FAILURE;
  }

  const nx_json* status = nx_json_get(out, "Status");
  if (! status) {
    e = err_string(0, "Missing status in JSON output");
    goto error;
  }

  if (status->type != NX_JSON_STRING) {
    e = err_string(0, "Status: not a JSON string");
    goto error;
  }

  if (strcmp(status->val.text, "OK")) {
    e = err_string(0, "Status != OK");
    goto error;
  }

error:
  nx_json_free(in);
  nx_json_free(out);
  Mem_Free(buf);

  if (e) {
    Log_Error("%s\n", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  return NBFC_EXIT_SUCCESS;
}

static char *to_lower(const char *a) {
  char* b = Mem_Strdup(a);
  for (char* c = b; *c; ++c)
    *c = tolower(*c);
  return b;
}

static bool str_starts_with_ignorecase(const char* string, const char* prefix) {
  for (;*prefix; prefix++, string++)
    if (tolower(*string) != tolower(*prefix))
      return false;
  return true;
}

static int levenshtein_min(int a, int b, int c) {
  if (a <= b && a <= c) {
    return a;
  }
  else if (b <= a && b <= c) {
    return b;
  }
  else {
    return c;
  }
}

static int levenshtein(const char *s1, const char *s2) {
  unsigned int x, y, s1len, s2len;
  s1len = strlen(s1);
  s2len = strlen(s2);
  unsigned int matrix[s2len+1][s1len+1];
  matrix[0][0] = 0;
  for (x = 1; x <= s2len; x++)
    matrix[x][0] = matrix[x-1][0] + 1;
  for (y = 1; y <= s1len; y++)
    matrix[0][y] = matrix[0][y-1] + 1;
  for (x = 1; x <= s2len; x++)
    for (y = 1; y <= s1len; y++)
      matrix[x][y] = levenshtein_min(matrix[x-1][y] + 1, matrix[x][y-1] + 1, matrix[x-1][y-1] + (s1[y-1] == s2[x-1] ? 0 : 1));

  return(matrix[s2len][s1len]);
}

static float word_difference(const char* s1, const char* s2) {
  const int s1len = strlen(s1);
  const int s2len = strlen(s2);
  const int diff = levenshtein(s1, s2);
  if (s1len > s2len)
    return 1.0f - ((float) diff / s1len);
  else
    return 1.0f - ((float) diff / s2len);
}
