#include "generated/client.help.h"
#include "nbfc.h"
#include "nxjson.c"
#include "nxjson.h"
#include "optparse/optparse.c"
#include "optparse/optparse.h"
#include "reverse_nxjson.c"
#include <ctype.h>
#include <dirent.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// check euid
static void check_root() {
  if (geteuid()) {
    fprintf(stderr, "This operation must be run as root\n");
    exit(NBFC_EXIT_CMDLINE);
  }
}

// get system model/product
static char *get_system_product() {
  check_root();
  FILE *proc = popen("dmidecode -s system-product-name", "r");
  struct timespec sleeptime = {.tv_nsec = 500000000L, .tv_sec = 0};
  nanosleep(&sleeptime, NULL);
  char *result = (char *)malloc(100);
  if (!fscanf(proc, "%[^\n]", result)) {
    fprintf(stderr, "Failed to read dmidecode output\n");
    exit(NBFC_EXIT_FAILURE);
  }
  int proc_exit_code = pclose(proc);
  char *error_msg = "Some error occurred";
  switch (proc_exit_code / 256) {
  case 0:
    return result;
  case 141:
    error_msg = "Timeout, dmidecode took too long to run";
    break;
  case 127:
    error_msg = "Can't find dmidecode, are you sure it's installed?";
    break;
  case 126:
    error_msg = "dmidecode isn't executable";
    break;
  }
  fprintf(stderr, "%s\ndmidecode exit code: %d\n", error_msg,
          proc_exit_code / 256);
  exit(NBFC_EXIT_CMDLINE);
}

// read pid of the service
static int get_service_pid() {
  FILE *pid_file = fopen(NBFC_PID_FILE, "r");
  if (pid_file) {
    int pid;
    if (!fscanf(pid_file, "%d", &pid)) {
      fprintf(stderr, "Failed to read the pid file\n");
      exit(NBFC_EXIT_FAILURE);
    }
    fclose(pid_file);
    return pid;
  }
  return -1;
}

// start the service
static void service_start(int readonly) {
  check_root();
  int pid = get_service_pid();
  if (pid != -1)
    printf("Service already running: %d\n", pid);
  else {
    char *cmd = malloc(32);
    strcpy(cmd, "nbfc_service -f ");
    strcat(cmd, readonly ? "-r" : "");
    int run = system(cmd);
    if (run == -1) {
      fprintf(stderr,
              "Can't run nbfc_service, make sure the binary is installed\n");
      exit(NBFC_EXIT_CMDLINE);
    }
  }
  exit(NBFC_EXIT_SUCCESS);
}

// stop the service
static void service_stop() {
  check_root();
  int pid = get_service_pid();
  if (pid == -1)
    return;
  kill(pid, SIGINT);
  remove(NBFC_STATE_FILE);
}

// restart the service
static void service_restart(int readonly) {
  check_root();
  service_stop();
  sleep(1);
  service_start(readonly);
}

struct ConfigFile {
  char *config_name;
  double diff;
};

struct ConfigList {
  struct ConfigFile *config_list;
  size_t list_size;
};

// get an array of config names
static struct ConfigList get_configs() {
  const int CONFIG_LIST_SIZE = 512;
  struct ConfigList files = {
      .config_list = malloc(CONFIG_LIST_SIZE * sizeof(struct ConfigFile)),
      .list_size = 0};
  DIR *configs;
  struct dirent *config;
  configs = opendir(NBFC_CONFIGS_DIR);
  if (configs == NULL) {
    fprintf(stderr, "The directory %s doesn't exist\n", NBFC_CONFIGS_DIR);
    exit(NBFC_EXIT_FAILURE);
  }
  config = readdir(configs); // .
  config = readdir(configs); // ..
  if (configs) {
    while ((config = readdir(configs)) != NULL) {
      *strchr(config->d_name, '.') = '\0'; // remove .json extension
      int config_l = strlen(config->d_name);
      files.config_list[files.list_size++].config_name =
          (char *)malloc(config_l);
      strncpy(files.config_list[files.list_size - 1].config_name,
              config->d_name, config_l);
    }
    closedir(configs);
  } else {
    fprintf(stderr, NBFC_CONFIGS_DIR " is not accessible\n");
    exit(NBFC_EXIT_CMDLINE);
  }
  return files;
}

// print config names
static void list_configs() {
  struct ConfigList files = get_configs();
  size_t f = -1;
  while (++f < files.list_size)
    printf("%s\n", files.config_list[f].config_name);
}

// recommend configs in a sorted array
static struct ConfigList recommended_configs() {
  int word_difference(char *, char *);
  float words_difference(char *, char *);
  char *to_lower(char *);
  char **sep(char *, char *);
  int compare_configs(const void *, const void *);
  char *product = get_system_product();
  struct ConfigList files = get_configs();
  size_t x = -1;
  while (++x < files.list_size)
    files.config_list[x].diff =
        words_difference(files.config_list[x].config_name, product);
  qsort(files.config_list, files.list_size, sizeof(struct ConfigFile),
        compare_configs);
  return files;
}

// compare function for qsort
int compare_configs(const void *a, const void *b) {
  return (((struct ConfigFile *)a)->diff - ((struct ConfigFile *)b)->diff);
}

// convert string to lowercase
char *to_lower(char *a) {
  char *b = (char *)malloc(strlen(a));
  for (int i = 0; a[i]; i++) {
    b[i] = tolower(a[i]);
  }
  return b;
}

// to separate a string by delims
char **sep(char *text, char *delims) {
  const int WORD_COUNT_LIMIT = 128;
  const int CHAR_COUNT_LIMIT = 256;
  char **words = malloc(sizeof(char *) * WORD_COUNT_LIMIT);
  int word_count = 0, char_count = 0;
  size_t i = 0;
  while (strchr(delims, text[i]))
    i++;
  for (; i < strlen(text); i++) {
    if (!char_count) {
      words[++word_count - 1] = malloc(CHAR_COUNT_LIMIT);
    }
    if (strchr(delims, text[i]) != NULL) {
      words[word_count - 1][char_count] = 0;
      char_count = 0;
    } else {
      words[word_count - 1][char_count++] = text[i];
    }
  }
  words[word_count - 1][char_count] = 0;
  words[word_count - 1] = NULL;
  return words;
}

// get sum of differences between characters of a word
int word_difference(char *a, char *b) {
  a = to_lower(a);
  b = to_lower(b);
  if (a == b)
    return 0;
  int diff = 0;
  for (size_t i = 0; i < min(strlen(a), strlen(b));
       i++, diff += abs(a[i] - b[i]))
    ;
  return diff;
}

// get sum of differences between words separated by a space
float words_difference(char *a, char *b) {
  char **words_a = sep(a, " "), **words_b = sep(b, " ");
  float diff = 0;
  int l = 0;
  while (*(words_a + l) != NULL && *(words_b + l) != NULL) {
    diff += word_difference(*(words_a + l), *(words_b + l));
    l++;
  }
  return diff / l;
}

static const nx_json *get_status() {
  FILE *state_file = fopen(NBFC_STATE_FILE, "r");
  if (state_file == NULL) {
    fprintf(stderr, "Service not running\n");
    exit(NBFC_EXIT_FAILURE);
  }
  fseek(state_file, 0, SEEK_END);
  long fsize = ftell(state_file);
  rewind(state_file);
  char *contents = malloc(fsize + 1);
  if (!fread(contents, 1, fsize, state_file)) {
    fprintf(stderr, "Failed to read the file\n");
    exit(NBFC_EXIT_FAILURE);
  }
  fclose(state_file);
  contents[fsize] = 0;
  return nx_json_parse_utf8(contents);
}

static const nx_json *get_config() {
  FILE *config_file = fopen(NBFC_SERVICE_CONFIG, "r");
  if (access(NBFC_SERVICE_CONFIG, F_OK) != 0) {
    return nx_json_parse_utf8("{}");
  }
  if (config_file == NULL) {
    fprintf(stderr, NBFC_SERVICE_CONFIG
            " is not accessible\n");
    exit(NBFC_EXIT_FAILURE);
  }
  fseek(config_file, 0, SEEK_END);
  long fsize = ftell(config_file);
  rewind(config_file);
  char *contents = malloc(fsize + 1);
  if (!fread(contents, 1, fsize, config_file)) {
    fprintf(stderr, "Failed to read the file\n");
    exit(NBFC_EXIT_FAILURE);
  }
  fclose(config_file);
  contents[fsize] = 0;
  return nx_json_parse_utf8(strcmp(contents, "") ? contents : "{}");
}

static void set_config(const nx_json *cfg) {
  check_root();
  FILE *state_file = fopen(NBFC_SERVICE_CONFIG, "w");
  if (state_file == NULL) {
    fprintf(stderr, NBFC_SERVICE_CONFIG
            " is not accessible\n");
    exit(NBFC_EXIT_FAILURE);
  }
  char *contents = nx_json_to_string(cfg);
  fprintf(state_file, "%s", contents);
  fclose(state_file);
}

static char *get_fan_status(const nx_json *fan) {
  char status[1024];
  snprintf(status, sizeof status,
          "Fan Display Name         : %s\n"
          "Auto Control Enabled     : %s\n"
          "Critical Mode Enabled    : %s\n"
          "Current Fan Speed        : %.2f\n"
          "Target Fan Speed         : %.2f\n"
          "Fan Speed Steps          : %lld\n",
          nx_json_get(fan, "name")->val.text,
          nx_json_get(fan, "automode")->val.i ? "true" : "false",
          nx_json_get(fan, "critical")->val.i ? "true" : "false",
          nx_json_get(fan, "current_speed")->val.dbl,
          nx_json_get(fan, "target_speed")->val.dbl,
          (long long) nx_json_get(fan, "speed_steps")->val.i);
  return strdup(status);
}

static const cli99_option main_options[] = {
    {"-v|--version", -'v', 0},
    {"-h|--help", -'h', 0},
    {"command", 'C', 1 | cli99_required_option},
    cli99_options_end()};

static const cli99_option status_command_options[] = {
    cli99_include_options(&main_options),
    {"-a|--all", -'a', 0},
    {"-s|--status", -'s', 0},
    {"-f|--fan", -'f', 1 | cli99_type_int},
    {"-w|--watch", -'w', 1 | cli99_type_int},
    cli99_options_end()};

static const cli99_option config_command_options[] = {
    cli99_include_options(&main_options),
    {"-l|--list", -'l', 0},
    {"-s|--set", -'s', 1 | cli99_required_option},
    {"-a|--apply", -'a', 1 | cli99_required_option},
    {"-r|--recommend", -'r', 0},
    cli99_options_end()};

static const cli99_option set_command_options[] = {
    cli99_include_options(&main_options),
    {"-a|--auto", -'a', 0},
    {"-s|--speed", -'s', 1 | cli99_type_float},
    {"-f|--fan", -'f', 1 | cli99_type_int},
    cli99_options_end()};

static const cli99_option start_command_options[] = {
    cli99_include_options(&main_options),
    {"-r|--readonly", -'r', 0},
    cli99_options_end()};

static const cli99_option dump_command_options[] = {
    cli99_include_options(&main_options), cli99_options_end()};

static const cli99_option *Options[] = {
    start_command_options,  dump_command_options,
    start_command_options, // restart
    status_command_options, config_command_options, set_command_options,
    dump_command_options,   dump_command_options,
};

enum Command {
  Command_Start,
  Command_Stop,
  Command_Restart,
  Command_Status,
  Command_Config,
  Command_Set,
  Command_Wait_For_Hwmon,
  Command_Help,
};

static struct {
  int fancount;
  int speedcount;
  int *fans;
  float *speeds;
  int a; // all/auto/apply
  const char *config;
  int l;     // list
  int r;     // recommend/readonly
  int help;  // help
  int s;     // set
  int watch; // watch time
} options;

static const char *HelpTexts[] = {
    CLIENT_START_HELP_TEXT,  CLIENT_DEFAULT_HELP("stop"),
    CLIENT_START_HELP_TEXT, // same optional args for restart for restart
    CLIENT_STATUS_HELP_TEXT, CLIENT_CONFIG_HELP_TEXT,
    CLIENT_SET_HELP_TEXT,    CLIENT_DEFAULT_HELP("wait-for-hwmon"),
    CLIENT_HELP_TEXT};

static int va_find_string(const char *needle, ...) {
  va_list va;
  va_start(va, needle);

  const char *s;
  for (int i = 0; (s = va_arg(va, const char *)); ++i) {
    if (!strcmp(s, needle)) {
      va_end(va);
      return i;
    }
  }
  va_end(va);
  return -1;
}

int main(int argc, char *const argv[]) {
  if (argc == 1) {
    printf(CLIENT_HELP_TEXT);
    return NBFC_EXIT_SUCCESS;
  }
  enum Command cmd = Command_Help;
  cli99 p;
  char o;
  options.fans = calloc(1, 1);
  options.speeds = calloc(1, 1);
  int size = 1;
  cli99_Init(&p, argc, argv, main_options, cli99_options_python);
  while ((o = cli99_GetOpt(&p))) {
    switch (o) {
    case 'C':
      cmd = (enum Command)va_find_string(p.optarg, "start", "stop", "restart",
                                         "status", "config", "set",
                                         "wait-for-hwmon", "help", 0);

      if (cmd == (enum Command) - 1)
        die(NBFC_EXIT_CMDLINE, "%s: Invalid command: %s\n", argv[0], p.optarg);

      if (cmd == Command_Help) {
        printf("%s", HelpTexts[cmd]);
        return NBFC_EXIT_SUCCESS;
      }
      cli99_SetOptions(&p, Options[cmd], false);
      break;

    case -'h':
      printf("%s", HelpTexts[cmd]);
      return NBFC_EXIT_SUCCESS;
    case -'v':
      printf("nbfc v" NBFC_VERSION "\n");
      return NBFC_EXIT_SUCCESS;
    case -'a':
      options.a = 1;
      if (cmd == Command_Config) {
        options.config = p.optarg;
      }
      if (cmd == Command_Set) {
        if (options.speedcount == size) {
          size *= 2;
          options.speeds = realloc(options.speeds, size * sizeof(float));
          options.fans = realloc(options.fans, size * sizeof(int));
        }
        options.speeds[++options.speedcount - 1] = -1;
      }
      break;
    case -'l':
      options.l = 1;
      break;
    case -'r':
      options.r = 1;
      break;
    case -'w':
      options.watch = p.optval.i;
      break;
    case -'f':
      if (cmd == Command_Set || cmd == Command_Status) {
        if (options.fancount == size) {
          size *= 2;
          options.fans = realloc(options.fans, size * sizeof(int));
          options.speeds = realloc(options.speeds, size * sizeof(float));
        }
        if (p.optval.i < 1) {
          fprintf(stderr, "The fan number can't be less than 1\n");
          return NBFC_EXIT_FAILURE;
        }
        options.fans[++options.fancount - 1] = p.optval.i;
      }
      break;
    case -'s':
      if (cmd == Command_Set) {
        if (options.speedcount == size) {
          size *= 2;
          options.speeds = realloc(options.speeds, size * sizeof(float));
          options.fans = realloc(options.fans, size * sizeof(int));
        }
        if (p.optval.f > 100) {
          fprintf(stderr,
                  "The value of the speed percentage cannot exceed 100\n");
          return NBFC_EXIT_FAILURE;
        }
        options.speeds[++options.speedcount - 1] = p.optval.f;
      } else {
        options.s = 1;
        if (cmd == Command_Config && options.a) {
          fprintf(stderr,
                  "You cannot use --apply and --set at the same time\n");
          return NBFC_EXIT_FAILURE;
        }
        options.config = p.optarg;
      }
      break;
    default:
      cli99_ExplainError(&p);
      exit(NBFC_EXIT_CMDLINE);
    }
  }
  switch (cmd) {
  case Command_Start:
    service_start(options.r);
    break;
  case Command_Stop:
    service_stop();
    break;
  case Command_Restart:
    service_restart(options.r);
    break;
  case Command_Config:
    if (options.l)
      list_configs();
    else if (options.r) {
      struct ConfigList files = recommended_configs();
      if (files.list_size && !files.config_list[0].diff) {
        printf("%s\n", files.config_list[0].config_name);
        return NBFC_EXIT_SUCCESS;
      } else {
        printf("No recommended config found\n");
        return NBFC_EXIT_FAILURE;
      }
    } else if (options.s || options.a) {
      check_root();
      char *model;
      if (!strcmp(options.config, "auto")) {
        struct ConfigList files = recommended_configs();
        if (files.list_size && !files.config_list[0].diff) {
          model = files.config_list[0].config_name;
          return NBFC_EXIT_SUCCESS;
        } else {
          fprintf(stderr,
                  "No recommended config found to apply automatically\n");
          return NBFC_EXIT_FAILURE;
        }
      } else {
        model = (char *)options.config;
        char *file = malloc(512);
        strcpy(file, NBFC_CONFIGS_DIR);
        strcat(file, "/");
        strcat(file, model);
        strcat(file, ".json");
        if (access(file, F_OK)) {
          fprintf(stderr, "No such config file found\n");
          return NBFC_EXIT_FAILURE;
        }
      }
      const nx_json *cfg = get_config();
      if (nx_json_get(cfg, "SelectedConfigId") == NULL) {
        int *cfg_len = (int *)&cfg->val.children.length;
        *cfg_len += 1;
        nx_json **first = (nx_json **)&cfg->val.children.first;
        nx_json *node = malloc(sizeof(nx_json));
        node->key = "SelectedConfigId";
        node->next = cfg->val.children.first;
        node->val.text = model;
        node->type = NX_JSON_STRING;
        *first = node;
      } else {
        char **textvalue =
            (char **)&nx_json_get(cfg, "SelectedConfigId")->val.text;
        *textvalue = model;
      }
      set_config(cfg);
      if (options.a)
        service_restart(0);
    } else {
      printf(CLIENT_CONFIG_HELP_TEXT);
      return NBFC_EXIT_SUCCESS;
    }
    break;
  case Command_Set: {
    if (!options.a && !options.fancount && !options.speedcount) {
      printf(CLIENT_SET_HELP_TEXT);
      return NBFC_EXIT_SUCCESS;
    }
    const nx_json *cfg = get_config();
    const nx_json *speedsjson = nx_json_get(cfg, "TargetFanSpeeds");
    int speedcount = 1;
    for (int i = 0; i < options.fancount; i++)
      speedcount = max(speedcount, options.fans[i]);
    if (speedsjson != NULL)
      speedcount = max(speedcount, speedsjson->val.children.length);

    double *speeds = malloc(sizeof(double) * speedcount);

    if (options.a && !options.speedcount && !options.fancount) {
      for (int i = 0; i < speedcount; i++)
        speeds[i] = -1;
    } else if (options.speedcount != 1 &&
               options.speedcount > options.fancount) {
      fprintf(stderr, "You need to provide speeds for every fan!\n");
      return NBFC_EXIT_FAILURE;
    } else if (options.speedcount == 1 && options.fancount == 0) {
      for (int i = 0; i < speedcount; i++)
        speeds[i] = options.speeds[0];
    } else {
      int break_ = speedsjson == NULL ? 0 : speedsjson->val.children.length;
      for (int i = 0; i < speedcount; i++)
        speeds[i] = i < break_
                        ? -2
                        : -1; // -2 for speeds that are not supposed to change
      for (int i = 0; i < options.speedcount; i++)
        speeds[options.fans[i] - 1] = options.speeds[i];
    }
    if (speedsjson != NULL) {
      int *speed_len = (int *)&speedsjson->val.children.length;
      *speed_len = speedcount;
      speedsjson = speedsjson->val.children.first;
      int k = 0;
      while (speedsjson != NULL) {
        if (speeds[k++] != -2) {
          double *speed = (double *)&speedsjson->val.dbl;
          *speed = speeds[k - 1];
        }
        if (speedsjson->next == NULL && k < speedcount) {
          nx_json *next = malloc(sizeof(nx_json));
          next->val.dbl = -1;
          next->key = NULL;
          next->next = NULL;
          next->type = NX_JSON_DOUBLE;
          nx_json **nextptr = (nx_json **)&speedsjson->next;
          *nextptr = next;
        }
        speedsjson = speedsjson->next;
      }
    } else {
      int k = 0;
      nx_json *next = malloc(sizeof(nx_json));
      next->type = NX_JSON_ARRAY;
      next->key = "TargetFanSpeeds";
      next->next = NULL;
      next->val.children.length = speedcount;
      nx_json *child = malloc(sizeof(nx_json));
      child->val.dbl = speeds[k++];
      child->key = NULL;
      child->next = NULL;
      child->type = NX_JSON_DOUBLE;
      next->val.children.first = child;
      int *cfg_len = (int *)&cfg->val.children.length;
      *cfg_len += 1;
      if (cfg->val.children.first == NULL) {
        nx_json **childptr = (nx_json **)&cfg->val.children.first;
        *childptr = next;
        speedsjson = cfg->val.children.first->val.children.first;
      } else {
        speedsjson = cfg->val.children.first;
        while (speedsjson->next != NULL)
          speedsjson = speedsjson->next;
        nx_json **nextptr = (nx_json **)&speedsjson->next;
        *nextptr = next;
        speedsjson = speedsjson->next->val.children.first;
      }
      while (k < speedcount) {
        nx_json *next = malloc(sizeof(nx_json));
        next->val.dbl = speeds[k++];
        next->key = NULL;
        next->next = NULL;
        next->type = NX_JSON_DOUBLE;
        nx_json **nextptr = (nx_json **)&speedsjson->next;
        *nextptr = next;
        speedsjson = speedsjson->next;
      }
    }
    set_config(cfg);
    service_restart(0);
    break;
  }
  case Command_Status: {
    while (true) {
      const nx_json *state = get_status();
      if (options.a || options.s) {
        printf("Read-only                : %s\n"
               "Selected Config Name     : %s\n"
               "Temperature              : %.2f\n",
               nx_json_get(state, "readonly")->val.i ? "true" : "false",
               nx_json_get(state, "config")->val.text,
               nx_json_get(state, "temperature")->val.dbl);
        if (options.a) {
          const nx_json *fan = nx_json_get(state, "fans")->val.children.first;
          while (fan != NULL) {
            printf("\n%s", get_fan_status(fan));
            fan = fan->next;
          }
        }
      } else if (options.fancount) {
        const nx_json *fans = nx_json_get(state, "fans");
        int fan_count = nx_json_get(state, "fans")->val.children.length;
        int *vis = calloc(sizeof(int), fan_count);
        for (int i = 0; i < options.fancount; i++) {
          if (options.fans[i] > fan_count) {
            fprintf(stderr, "Fan number %d not found!\n", options.fans[i]);
            return NBFC_EXIT_FAILURE;
          }
          if (!vis[options.fans[i] - 1]) {
            printf("%s\n", get_fan_status(nx_json_item(fans, options.fans[i] - 1)));
            vis[options.fans[i] - 1] = 1;
          }
        }
      } else {
        printf(CLIENT_STATUS_HELP_TEXT);
        return NBFC_EXIT_SUCCESS;
      }
      if (!options.watch)
        break;
      sleep(options.watch);
      fputs("\033c", stdout);
    }
    break;
  }
  case Command_Wait_For_Hwmon: {
    enum { file_names_length = 2, sensor_names_length = 3 };
    char *hwmon_file_names[file_names_length] = {
        "/sys/class/hwmon/hwmon%d/name",
        "/sys/class/hwmon/hwmon%d/device/name"};
    char *linux_temp_sensor_names[sensor_names_length] = {
        "coretemp", "k10temp", "zenpower"};
    for (int _ = 0; _ < 30; _++) {
      for (int i = 0; i < file_names_length; i++) {
        for (int j = 0; j < 10; j++) {
          char *file_name = malloc(strlen(hwmon_file_names[i]));
          sprintf(file_name, hwmon_file_names[i], j);
          FILE *file = fopen(file_name, "r");
          if (file == NULL)
            continue;
          fseek(file, 0, SEEK_END);
          int fsize = ftell(file);
          rewind(file);
          char *contents = malloc(fsize + 1);
          if (!fread(contents, 1, fsize, file)) {
            fprintf(stderr, "Failed to read the file\n");
            return NBFC_EXIT_FAILURE;
          }
          // trim the newline
          contents[strcspn(contents, "\n")] = '\0';
          for (int k = 0; k < sensor_names_length; k++) {
            if (!strcmp(contents, linux_temp_sensor_names[k])) {
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
  default:
    break;
  }
  return NBFC_EXIT_SUCCESS;
}
