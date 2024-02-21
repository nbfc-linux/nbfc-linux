#define _XOPEN_SOURCE 500 /* unistd.h: export pwrite()/pread(), string.h: export strdup */
#define _DEFAULT_SOURCE   /* endian.h: */

#define NX_JSON_CALLOC(SIZE) ((nx_json*) Mem_Calloc(1, SIZE))
#define NX_JSON_FREE(JSON)   (Mem_Free((void*) (JSON)))

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <locale.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "generated/client.help.h"
#include "log.h"
#include "macros.h"
#include "nbfc.h"
#include "nxjson_utils.h"
#include "parse_number.h"
#include "parse_double.h"
#include "slurp_file.h"
#include "stringbuf.h"

#include "error.c"
#include "optparse/optparse.c"
#include "model_config.c"
#include "memory.c"
#include "program_name.c"
#include "nxjson.c"
#include "reverse_nxjson.c"

#define DmiIdDirectoryPath "/sys/devices/virtual/dmi/id"

static ServiceInfo service_info;
static ServiceConfig service_config;

static const cli99_option main_options[] = {
  {"-v|--version",  -'v', 0},
  {"-h|--help",     -'h', 0},
  {"command",        'C', 1 | cli99_required_option},
  cli99_options_end()
};

static const cli99_option status_command_options[] = {
  cli99_include_options(&main_options),
  {"-a|--all",      -'a', 0},
  {"-s|--service",  -'s', 0},
  {"-f|--fan",      -'f', 1},
  {"-w|--watch",    -'w', 1},
  cli99_options_end()
};

static const cli99_option config_command_options[] = {
  cli99_include_options(&main_options),
  {"-l|--list",       -'l', 0},
  {"-s|--set",        -'s', 1},
  {"-a|--apply",      -'a', 1},
  {"-r|--recommend",  -'r', 0},
  cli99_options_end()
};

static const cli99_option set_command_options[] = {
  cli99_include_options(&main_options),
  {"-a|--auto",       -'a', 0},
  {"-s|--speed",      -'s', 1},
  {"-f|--fan",        -'f', 1},
  cli99_options_end()
};

static const cli99_option start_command_options[] = {
  cli99_include_options(&main_options),
  {"-r|--read-only",  -'r', 0},
  cli99_options_end()
};

static const cli99_option *Options[] = {
  start_command_options,
  main_options,
  start_command_options, // restart
  status_command_options,
  config_command_options,
  set_command_options,
  main_options,
  main_options,
  main_options,
};

static const char *HelpTexts[] = {
  CLIENT_START_HELP_TEXT,
  CLIENT_STOP_HELP_TEXT,
  CLIENT_RESTART_HELP_TEXT,
  CLIENT_STATUS_HELP_TEXT,
  CLIENT_CONFIG_HELP_TEXT,
  CLIENT_SET_HELP_TEXT,
  CLIENT_WAIT_FOR_HWMON_HELP_TEXT,
  CLIENT_GET_MODEL_HELP_TEXT,
  CLIENT_HELP_TEXT
};

enum Command {
  Command_Start,
  Command_Stop,
  Command_Restart,
  Command_Status,
  Command_Config,
  Command_Set,
  Command_Wait_For_Hwmon,
  Command_Get_Model_Name,
  Command_Help,
};

static enum Command Command_From_String(const char* s) {
  const char* commands[] = {
    "start", "stop", "restart", "status", "config", "set",
    "wait-for-hwmon", "get-model-name", "help"
  };

  for (int i = 0; i < ARRAY_SSIZE(commands); ++i)
    if (!strcmp(commands[i], s))
      return (enum Command) i;

  return (enum Command) -1;
}

static struct {
  array_of(int) fans;
  array_of(float) speeds;
  int a; // all/auto/apply
  const char *config;
  int l;     // list
  int r;     // recommend/read-only
  int s;     // set
  int watch; // watch time
} options = {0};

static int Service_Start(bool);
static int Service_Stop();
static int Service_Restart(bool);
static int Config();
static int Set();
static int Status();
static int Wait_For_Hwmon();
static int Get_Model_Name();

int main(int argc, char *const argv[]) {
  if (argc == 1) {
    printf(CLIENT_HELP_TEXT);
    return NBFC_EXIT_CMDLINE;
  }

  Program_Name_Set(argv[0]);
  setlocale(LC_NUMERIC, "C"); // for json floats
  mkdir(NBFC_CONFIG_DIR, 0755);

  int o;
  char* err;
  enum Command cmd = Command_Help;
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

    case -'h':
      printf("%s", HelpTexts[cmd]);
      return NBFC_EXIT_SUCCESS;
    case -'v':
      printf("nbfc " NBFC_VERSION "\n");
      return NBFC_EXIT_SUCCESS;
    case -'a':
      options.a = 1;
      if (cmd == Command_Config)
        options.config = p.optarg;
      break;
    case -'l':
      options.l = 1;
      break;
    case -'r':
      options.r = 1;
      break;
    case -'w':
      options.watch = parse_number(p.optarg, 1, INT64_MAX, &err);
      if (err) {
        Log_Error("-w|--watch: %s\n", err);
        return NBFC_EXIT_FAILURE;
      }
      break;
    case -'f':
      if (cmd == Command_Set || cmd == Command_Status) {
        const int fan = parse_number(p.optarg, 0, INT64_MAX, &err);
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
        options.s = 1;
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
  default: break;
  }

  return NBFC_EXIT_SUCCESS;
}

static char*  to_lower(const char*);
static bool   str_starts_with_ignorecase(const char*, const char*);
static char** get_longest_common_substrings(const char*, const char*);
static char*  get_longest_common_substring(const char*, const char*);
static float  get_similarity_index(const char*, const char*);

static const char* get_system_product() {
  static char buf[128] = {0};
  if (*buf)
    return buf;

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
  static char buf[128] = {0};
  if (*buf)
    return buf;

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
  static char model_name[256] = {0};
  if (*model_name)
    return model_name;

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
  char buf[32];
  if (slurp_file(buf, sizeof(buf), NBFC_PID_FILE) == -1)
    return -1;

  char* endptr;
  errno = 0;
  int pid = strtol(buf, &endptr, 10);

  if (errno || endptr == buf) {
    Log_Error("Failed to read the pid file: " NBFC_PID_FILE "\n");
    exit(NBFC_EXIT_FAILURE);
  }

  return pid;
}

static int Service_Start(bool read_only) {
  check_root();
  int pid = get_service_pid();
  if (pid != -1)
    Log_Info("Service already running (pid: %d)\n", pid);
  else {
    char cmd[32] = "nbfc_service -f";
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
  return NBFC_EXIT_SUCCESS;
}

static int Service_Stop() {
  check_root();
  int pid = get_service_pid();
  if (pid == -1) {
    Log_Error("Service not running\n");
    return NBFC_EXIT_SUCCESS;
  }
  remove(NBFC_STATE_FILE);
  remove(NBFC_PID_FILE);
  Log_Info("Killing nbfc_service (%d)\n", pid);
  if (kill(pid, SIGINT) == -1) {
    Log_Error("Failed to kill nbfc_service process (%d): %s\n", pid, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }
  return NBFC_EXIT_SUCCESS;
}

static int Service_Restart(bool read_only) {
  check_root();
  Service_Stop();
  return Service_Start(read_only);
}

struct ConfigFile {
  char *config_name;
  double diff;
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

  struct dirent *file;
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
    file->diff = get_similarity_index(product, file->config_name);
  }
  qsort(files.data, files.size, sizeof(struct ConfigFile), compare_config_by_diff);
  return files;
}

static void ServiceInfo_Load() {
  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;
  Error* e = nx_json_parse_file(&js, buf, sizeof(buf), NBFC_STATE_FILE);
  if (e)
    goto error;

  e = ServiceInfo_FromJson(&service_info, js);
  nx_json_free(js);

  if (e)
    goto error;

  e = ServiceInfo_ValidateFields(&service_info);
  if (e)
    goto error;

  for_each_array(FanInfo*, f, service_info.fans) {
    e = FanInfo_ValidateFields(f);
    if (e)
      goto error;
  }

  if (e) {
error:
    e = err_string(e, NBFC_STATE_FILE);
    e_die();
  }
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

static void ServiceConfig_Write() {
  check_root();
  int fd = open(NBFC_SERVICE_CONFIG, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if (fd == -1) {
    Log_Error("Could not open " NBFC_SERVICE_CONFIG ": %s\n", strerror(errno));
    exit(NBFC_EXIT_FAILURE);
  }

  nx_json root = {0};
  nx_json *o = create_json(NX_JSON_OBJECT, NULL, &root);

  if (service_config.SelectedConfigId != NULL) {
    nx_json *o1 = create_json(NX_JSON_STRING, "SelectedConfigId", o);
    o1->val.text = service_config.SelectedConfigId;
  }

  if (service_config.EmbeddedControllerType != EmbeddedControllerType_Unset) {
    nx_json *o1 = create_json(NX_JSON_STRING, "EmbeddedControllerType", o);
    o1->val.text = EmbeddedControllerType_ToString(service_config.EmbeddedControllerType);
  }

  if (service_config.TargetFanSpeeds.size) {
    nx_json *o1  = create_json(NX_JSON_ARRAY, "TargetFanSpeeds", o);

    for_each_array(float*, f, service_config.TargetFanSpeeds) {
      nx_json *o2 = create_json(NX_JSON_DOUBLE, NULL, o1);
      o2->val.dbl = *f;
    }
  }

  char buf[NBFC_MAX_FILE_SIZE];
  StringBuf s = { buf, 0, sizeof(buf) };
  buf[0] = 0;

  nx_json_to_string(o, &s, 0);
  write(fd, s.s, s.size);
  close(fd);
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
    sleep(1);
  }

  return NBFC_EXIT_FAILURE;
}

static int Get_Model_Name() {
  printf("%s\n", get_model_name());
  return NBFC_EXIT_SUCCESS;
}

static void print_fan_status(const FanInfo* fan) {
  printf("Fan Display Name         : %s\n"
         "Auto Control Enabled     : %s\n"
         "Critical Mode Enabled    : %s\n"
         "Current Fan Speed        : %.2f\n"
         "Target Fan Speed         : %.2f\n"
         "Fan Speed Steps          : %d\n",
         fan->name,
         fan->automode ? "true" : "false",
         fan->critical ? "true" : "false",
         fan->current_speed,
         fan->target_speed,
         fan->speed_steps);
}

static void print_service_status() {
  printf("Read-only                : %s\n"
         "Selected Config Name     : %s\n"
         "Temperature              : %.2f\n",
         service_info.read_only ? "true" : "false",
         service_info.config,
         service_info.temperature);
}

static int Status() {
  if (!options.s && !options.a && !options.fans.size)
    options.a = 1;

  while (true) {
    ServiceInfo_Load();

    if (options.a || options.s) {
      print_service_status();
      if (options.a) {
        for_each_array(const FanInfo*, f, service_info.fans) {
          printf("\n");
          print_fan_status(f);
        }
      }
    }

    if (options.fans.size) {
      const int fan_count = service_info.fans.size;
      bool *vis = Mem_Calloc(sizeof(bool), fan_count);
      for_each_array(int*, fan_index, options.fans) {
        if (*fan_index > fan_count - 1) {
          Log_Error("Fan number %d not found! (Fan indexes count from zero!)\n", *fan_index);
          return NBFC_EXIT_FAILURE;
        }
        if (!vis[*fan_index]) {
          printf("\n");
          print_fan_status(&service_info.fans.data[*fan_index]);
          vis[*fan_index] = 1;
        }
      }
      Mem_Free(vis);
    }

    if (!options.watch)
      break;
    sleep(options.watch);
    fputs("\033c", stdout);
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
    array_of(ConfigFile) files = recommended_configs();
    if (files.size && !strcmp(files.data[0].config_name, get_model_name())) {
      printf("%s\n", files.data[0].config_name);
    } else {
      for_each_array(ConfigFile*, file, files) {
        if (file->diff >= 1.0f)
          printf("%s\n", file->config_name);
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
    ServiceConfig_Write();

    if (options.a)
      return Service_Restart(0);
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

  ServiceInfo_Load();
  ServiceConfig_Load();

  if (! options.fans.size) {
    options.fans.data = Mem_Malloc(sizeof(int) * service_info.fans.size);
    for (int i = 0; i < service_info.fans.size; ++i)
      options.fans.data[i] = i;
    options.fans.size  = service_info.fans.size;
  }

  const int fancount = service_info.fans.size;
  float* speeds = Mem_Malloc(sizeof(float) * fancount);

  for (int i = 0; i < fancount; ++i)
    speeds[i] = -1;
  for (int i = 0; i < min(service_config.TargetFanSpeeds.size, fancount); ++i)
    speeds[i] = service_config.TargetFanSpeeds.data[i];

  for_each_array(int*, fan_index, options.fans) {
    if (*fan_index > fancount - 1) {
      Log_Error("Fan number %d not found! (Fan indexes count from zero!)\n", *fan_index);
      return NBFC_EXIT_FAILURE;
    }

    speeds[*fan_index] = options.a ? -1 : options.speeds.data[0];
  }

  service_config.TargetFanSpeeds.data = speeds;
  service_config.TargetFanSpeeds.size = fancount;

  ServiceConfig_Write();
  return Service_Restart(0);
}

static char *to_lower(const char *a) {
  char *b = (char *)Mem_Malloc(strlen(a) + 1);
  int i;
  for (i = 0; a[i]; i++) {
    b[i] = tolower(a[i]);
  }
  b[i] = '\0';
  return b;
}

static bool str_starts_with_ignorecase(const char* string, const char* prefix) {
  for (;*prefix; prefix++, string++)
    if (tolower(*string) != tolower(*prefix))
      return false;
  return true;
}

static char** get_longest_common_substrings(const char* str1, const char* str2) {
  if (!str1 || !str2) {
    return NULL;
  }

  int len1 = strlen(str1);
  int len2 = strlen(str2);
  int** lookup = (int**)Mem_Calloc(len1 + 1, sizeof(int*));
  for (int i = 0; i <= len1; i++) {
    lookup[i] = (int*)Mem_Calloc(len2 + 1, sizeof(int));
  }

  int max_len = 0;
  int* end_indices = NULL;
  int num_end_indices = 0;

  for (int i = 1; i <= len1; i++) {
    for (int j = 1; j <= len2; j++) {
      if (str1[i - 1] == str2[j - 1]) {
        lookup[i][j] = lookup[i - 1][j - 1] + 1;
        if (lookup[i][j] > max_len) {
          max_len = lookup[i][j];
          num_end_indices = 1;
          end_indices = (int*)Mem_Realloc(end_indices, sizeof(int));
          end_indices[0] = i;
        } else if (lookup[i][j] == max_len) {
          num_end_indices++;
          end_indices = (int*)Mem_Realloc(end_indices, num_end_indices * sizeof(int));
          end_indices[num_end_indices - 1] = i;
        }
      }
    }
  }

  char** substrings = (char**)Mem_Calloc(num_end_indices + 1, sizeof(char*));
  for (int i = 0; i < num_end_indices; i++) {
    substrings[i] = (char*)Mem_Calloc(max_len + 1, sizeof(char));
    strncpy(substrings[i], &str1[end_indices[i] - max_len], max_len);
  }
  substrings[num_end_indices] = NULL;

  for (int i = 0; i <= len1; i++) {
    Mem_Free(lookup[i]);
  }
  Mem_Free(lookup);
  Mem_Free(end_indices);

  return substrings;
}

static char* get_longest_common_substring(const char* str1, const char* str2) {
  char** strings = get_longest_common_substrings(str1, str2);
  if (strings == NULL)
    return NULL;

  char* ret = NULL;
  char** string = strings;

  if (*string) {
    ret = *string;
    ++string;
  }

  for (; *string; ++string) {
    Mem_Free(*string);
  }
  Mem_Free(strings);
  return ret;
}

static char **split(const char *str) {
  int num_words = 0;
  const char *p = str;

  // Count words in str
  while (*p) {
    while (*p == ' ') {
      p++;
    }
    if (*p) {
      num_words++;
      while (*p && (*p != ' ')) {
        p++;
      }
    }
  }

  char **words = Mem_Malloc((num_words + 1) * sizeof(char *));
  int i = 0;
  p = str;
  while (*p) {
    while (*p == ' ') {
      p++;
    }
    if (*p) {
      const char *start = p;
      while (*p && (*p != ' ')) {
        p++;
      }
      const int len = p - start;
      words[i] = Mem_Malloc((len + 1) * sizeof(char));
      strncpy(words[i], start, len);
      words[i][len] = '\0';
      i++;
    }
  }

  words[i] = NULL;

  return words;
}

static float get_similarity_index(const char* model_name1, const char* model_name2) {
  float result = 0;
  char* model_name1_lower = to_lower(model_name1);
  char* model_name2_lower = to_lower(model_name2);

  char** model_name1_splitted = split(model_name1_lower);
  char** model_name2_splitted = split(model_name2_lower);

  for (char** s1 = model_name1_splitted; *s1; ++s1) {
    float max_similarity = 0;

    for (char** s2 = model_name2_splitted; *s2; ++s2) {
      char* lcs = get_longest_common_substring(*s1, *s2);
      int lcs_length = lcs ? strlen(lcs) : 0;
      Mem_Free(lcs);

      if (lcs_length < 2)
        continue;

      float similarity = (float)lcs_length / (float)max(strlen(*s1), strlen(*s2));
      max_similarity = fmax(max_similarity, similarity);
    }

    result += max_similarity;
  }

  Mem_Free(model_name1_lower);
  Mem_Free(model_name2_lower);
  for (char** s = model_name1_splitted; *s; ++s) Mem_Free(*s);
  for (char** s = model_name2_splitted; *s; ++s) Mem_Free(*s);
  Mem_Free(model_name1_splitted);
  Mem_Free(model_name2_splitted);

  return result;
}
