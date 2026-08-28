#include "../nbfc.h"
#include "../error.h"
#include "../memory.h"
#include "../acpi_analysis.h"
#include "../config_rating.h"
#include "../file_utils.h"
#include "../model_config_utils.h"
#include "../nxjson_utils.h"

#include <string.h> // memset, strerror
#include <linux/limits.h>

#include "curl_utils.h"
#include "config_files.h"
#include "check_root.h"
#include "client_global.h"

#define RATE_CONFIG_RECOMMENDED_MINIMUM_SCORE 9.0
#define RATE_CONFIG_REGIONS_MAX               4

#define RATE_CONFIG_RULES_JSON_URL \
  "https://raw.githubusercontent.com/nbfc-linux/nbfc-linux/main/endpoints/config_rating_rules_v2.json"

const struct cli99_Option RateConfig_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"-d|--dsdt",        Option_RateConfig_DSDT_File,   cli99_RequiredArgument},
  {"-D|--dsdt-dir",    Option_RateConfig_DSDT_Dir,    cli99_RequiredArgument},
  {"-a|--all",         Option_RateConfig_All,         cli99_NoArgument      },
  {"-H|--full-help",   Option_RateConfig_Full_Help,   cli99_NoArgument      },
  {"-j|--json",        Option_RateConfig_Json,        cli99_NoArgument      },
  {"-m|--min-score",   Option_RateConfig_Min_Score,   cli99_RequiredArgument},
  {"-n|--no-download", Option_RateConfig_No_Download, cli99_NoArgument      },
  {"-r|--rules",       Option_RateConfig_Rules,       cli99_RequiredArgument},
  {"-i|--input",       Option_RateConfig_Input,       cli99_RequiredArgument},
  {"-u|--unverified",  Option_RateConfig_Unverified,  cli99_NoArgument      },
  {"-b|--bad",         Option_RateConfig_Bad,         cli99_NoArgument      },
  {"-q|--quiet",       Option_RateConfig_Quiet,       cli99_NoArgument      },
  {"-f|--fan-count",   Option_RateConfig_FanCount,    cli99_RequiredArgument},
  {"--print-rules",    Option_RateConfig_Print_Rules, cli99_NoArgument      },
  {"file",             Option_RateConfig_File,        cli99_NormalPositional},
  cli99_Options_End()
};

enum NBFC_PACKED_ENUM RateConfig_Action {
  RateConfig_Action_None,
  RateConfig_Action_RateAll,
  RateConfig_Action_RateFromFile,
  RateConfig_Action_RateFile,
  RateConfig_Action_PrintRules,
  RateConfig_Action_PrintFullHelp,
};

enum NBFC_PACKED_ENUM RateConfig_PrintStyle {
  RateConfig_PrintName,
  RateConfig_PrintNameAndScore,
  RateConfig_PrintFull,
};

enum NBFC_PACKED_ENUM RateConfig_Filter {
  RateConfig_FilterBadOnly,
  RateConfig_FilterGoodOnly,
  RateConfig_FilterAll,
};

struct {
  const char* action_option_string;
  enum RateConfig_Action action;
  enum RateConfig_PrintStyle style;
  enum RateConfig_Filter filter;
  bool        json;
  bool        no_download;
  bool        min_score_set;
  bool        unverified;
  uint8_t     fan_count;
  float       min_score;
  const char* file;
  const char* dsdt_files[ACPI_ANALYSIS_MAX_AML_FILES];
  size_t      dsdt_files_size;
  const char* dsdt_dir;
  const char* rules_file;
  const char* input_file;
} RateConfig_Options = {
  NULL,
  RateConfig_Action_None,
  RateConfig_PrintFull,
  RateConfig_FilterGoodOnly,
  false,
  false,
  false,
  false,
  0,
  RATE_CONFIG_RECOMMENDED_MINIMUM_SCORE,
  NULL,
  {0},
  0,
  NULL,
  NULL,
  NULL,
};

void RateConfig_SetAction(enum RateConfig_Action action, const char* option) {
  if (RateConfig_Options.action != RateConfig_Action_None) {
    Log_Error("%s cannot be used with %s", RateConfig_Options.action_option_string, option);
    exit(NBFC_EXIT_CMDLINE);
  }

  RateConfig_Options.action = action;
  RateConfig_Options.action_option_string = option;
}

/*
 * Download config rating rules from the repository.
 */
static Error RateConfig_DownloadRules(char** out) {
  Error e = err_success();
  CURL* curl = CurlWithMem_Create(RATE_CONFIG_RULES_JSON_URL, NULL);
  CURLcode code;
  long http_code;
  *out = NULL;

  code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    Log_Download_Failed(RATE_CONFIG_RULES_JSON_URL, code);
    e = err_string("Download failed");
    goto end;
  }

  code = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if (code != CURLE_OK) {
    Log_Error("curl_easy_getinfo() failed");
    e = err_string("Download failed");
    goto end;
  }

  if (http_code != 200) {
    Log_Error("Download failed: %s (server returned HTTP %ld)",
      RATE_CONFIG_RULES_JSON_URL, http_code);
    e = err_string("Download failed");
    goto end;
  }

  // Success
  *out = CurlWithMem_StealData(curl);

end:
  CurlWithMem_Destroy(curl);
  return e;
}

/*
 * Return config rating rules.
 */
static char* RateConfig_GetRules(const char* rules_file, bool no_download) {
  Error e;
  char* out;

  if (rules_file) {
    if (! slurp_file_dynamic(&out, rules_file).ok) {
      Log_Error("%s: %s", rules_file, strerror(errno));
      return NULL;
    }

    return out;
  }

  if (! no_download) {
    e = RateConfig_DownloadRules(&out);
    if (e)
      Log_Warn("Falling back on builtin configuration rating rules");
    else
      return out;
  }

  return Mem_Strdup(CONFIG_RATING_DEFAULT_RULES);
}

struct ConfigWithData {
  char* file;
  array_size_t group_id;
  ModelConfig model_config;
  ConfigRating_Rating rating;
};
typedef struct ConfigWithData ConfigWithData;
declare_array_of(ConfigWithData);

void ConfigWithData_Free(ConfigWithData* config_with_data) {
  Mem_Free(config_with_data->file);
  ModelConfig_Free(&config_with_data->model_config);
  ConfigRating_RatingFree(&config_with_data->rating);
  memset(config_with_data, 0, sizeof(*config_with_data));
}

static void PrintFullHelp(void) {
  puts(
    "nbfc rate-config evaluates one or more NBFC configuration files.\n"
    "\n"
    "The command analyzes whether a configuration can be executed safely\n"
    "on the current system. A positive rating does not necessarily mean\n"
    "that the configuration will work correctly, only that it appears\n"
    "reasonable and non-destructive.\n"
    "\n"
    "Configurations that reference the same registers and ACPI methods\n"
    "are grouped together.\n"
    "\n"
    "Ratings glossary:\n"
    "\n"
    "  FULL MATCH\n"
    "    Indicates that the register is a known fan register.\n"
    "\n"
    "  PARTIAL MATCH\n"
    "    Indicates that the register name contains FAN, RPM, or PWM.\n"
    "\n"
    "  MINIMAL MATCH\n"
    "    Indicates that the register name starts with the letter 'F'.\n"
    "\n"
    "  BAD REGISTER\n"
    "    Indicates that the register name starts with the letter 'B'\n"
    "    and is likely a battery-related register.\n"
    "\n"
    "  NO MATCH\n"
    "    Indicates that none of the matching rules apply.\n"
    "\n"
    "  NOT FOUND\n"
    "    Indicates that the register is not named in the firmware and\n"
    "    additional information could not be retrieved.\n"
    "\n"
    "For fan registers, at least a MINIMAL MATCH is required to consider\n"
    "a configuration usable.\n"
    "\n"
    "For RegisterWriteConfiguration registers, some registers may not yet\n"
    "be present in the rule database. In these cases, a NO MATCH result\n"
    "may still be acceptable.\n"
    "\n"
    "If in doubt, it is recommended to dump the firmware using\n"
    "`sudo nbfc acpi-dump dsl` and manually analyze the registers used\n"
    "by the configuration file. This requires some technical knowledge."
  );
}

/*
 * Rate all config files found in `files`.
 *
 * For each file:
 * - The filename is set (stored in ConfigWithData.file)
 * - The model configuration is loaded (stored in ConfigWithData.model_config)
 * - The model configuration is rated (stored in ConfigWithData.rating)
 */
static array_of(ConfigWithData) RateConfig_RateConfigs(
  ConfigRating* config_rating,
  array_of(ConfigFile)* files
) {
  Error e;
  char path[PATH_MAX];
  array_of(ConfigWithData) result;

  // Allocate memory for result
  result.size = 0;
  result.data = Mem_Calloc(files->size, sizeof(ConfigWithData));

  for_each_array(ConfigFile*, file, *files) {
    Trace trace = {0};
    ConfigWithData* config_with_data = &result.data[result.size];

    // Load the configuration file
    e = ModelConfig_FindAndLoad(&config_with_data->model_config, path, file->config_name);
    if (e) {
      Log_Warn("%s: %s", path, err_print_all(e));
      continue;
    }

    // Validate the configuration data (and silence warnings)
    Trace_Push(&trace, "%s", path);
    LogLevel old = Log_LogLevel;
    Log_LogLevel = LogLevel_Quiet;
    e = ModelConfig_Validate(&trace, &config_with_data->model_config);
    Log_LogLevel = old;
    if (e) {
      Log_Warn("%s", err_print_all(e));
      ModelConfig_Free(&config_with_data->model_config);
      continue;
    }

    // Rate the config
    e = ConfigRating_RateModelConfig(
      config_rating,
      &config_with_data->model_config,
      &config_with_data->rating
    );
    if (e) {
      Log_Warn("%s: %s", path, err_print_all(e));
      ModelConfig_Free(&config_with_data->model_config);
      continue;
    }

    // Set the filename
    config_with_data->file = Mem_Strdup(file->config_name);

    result.size++;
  }

  return result;
}

/*
 * Groups an array of ConfigWithData by model similarity.
 *
 * Grouping is performed by setting ConfigWithData.group_id.
 *
 * Returns the number of groups.
 */
static array_size_t RateConfig_GroupRatingsBySimilarConfig(array_of(ConfigWithData)* configs) {
  const array_size_t group_unset = (array_size_t) -1;
  array_size_t next_group = 0;

  for_each_array(ConfigWithData*, config, *configs)
    config->group_id = group_unset;

  for_enumerate_array(array_size_t, i, *configs) {
    if (configs->data[i].group_id != group_unset)
      continue;

    configs->data[i].group_id = next_group;

    for (array_size_t j = i + 1; j < configs->size; ++j) {
      if (configs->data[j].group_id != group_unset)
        continue;

      const bool similar = ModelConfig_IsSimilar(
          &configs->data[i].model_config,
          &configs->data[j].model_config);

      const bool same_rating = (configs->data[i].rating.score == configs->data[j].rating.score);

      if (similar && same_rating)
        configs->data[j].group_id = next_group;
    }

    ++next_group;
  }

  return next_group;
}

static void RateConfig_SortResultByScore(array_of(ConfigWithData)* result) {
  if (! result->size)
    return;

  /* Bubble sort - ascending */
  for (array_size_t i = 0; i < result->size - 1; ++i) {
    for (array_size_t j = 0; j < result->size - i - 1; ++j) {
      ConfigWithData* a = &result->data[j];
      ConfigWithData* b = &result->data[j + 1];
      if (a->rating.score > b->rating.score) {
        const ConfigWithData swap = *a;
        *a = *b;
        *b = swap;
      }
    }
  }
}

static void RateConfig_SortResultByPriority(array_of(ConfigWithData)* result) {
  if (! result->size)
    return;

  /* Bubble sort - ascending */
  for (array_size_t i = 0; i < result->size - 1; ++i) {
    for (array_size_t j = 0; j < result->size - i - 1; ++j) {
      ConfigWithData* a = &result->data[j];
      ConfigWithData* b = &result->data[j + 1];
      if (a->rating.priority > b->rating.priority) {
        const ConfigWithData swap = *a;
        *a = *b;
        *b = swap;
      }
    }
  }
}

static bool RateConfig_GroupHasMinScore(
  array_of(ConfigWithData)* results,
  array_size_t group_id,
  float min_score
) {
  for_each_array(ConfigWithData*, result, *results) {
    if (result->group_id == group_id)
      return (result->rating.score >= min_score);
  }

  return false;
}

static bool RateConfig_GroupHasFanCount(
  array_of(ConfigWithData)* results,
  array_size_t group_id
) {
  if (! RateConfig_Options.fan_count)
    return true;

  for_each_array(ConfigWithData*, result, *results) {
    if (result->group_id == group_id)
      return (result->model_config.FanConfigurations.size == RateConfig_Options.fan_count);
  }

  return false;
}

static bool RateConfig_GroupFilterBad(
  array_of(ConfigWithData)* results,
  array_size_t group_id,
  enum RateConfig_Filter filter
) {
  for_each_array(ConfigWithData*, result, *results) {
    if (result->group_id == group_id) {
      switch (filter) {
      case RateConfig_FilterBadOnly:  return result->rating.bad;
      case RateConfig_FilterGoodOnly: return !result->rating.bad;
      case RateConfig_FilterAll:      return true;
      }
    }
  }

  return true;
}

static void RateConfig_PrintResultGroup(
  array_of(ConfigWithData)* results,
  array_size_t group_id
) {
  ConfigWithData* last_result = NULL;

  for_each_array(ConfigWithData*, result, *results) {
    if (result->group_id != group_id)
      continue;

    switch (RateConfig_Options.style) {
      case RateConfig_PrintFull:
      case RateConfig_PrintName:
        printf("%s\n", result->file);
        break;
      case RateConfig_PrintNameAndScore:
        printf("%s (%.2f/10)\n", result->file, result->rating.score);
        break;
    }

    last_result = result;
  }

  if (RateConfig_Options.style == RateConfig_PrintFull)
    ConfigRating_RatingPrint(&last_result->rating);
}

static void RateConfig_PrintResults(
  array_of(ConfigWithData)* results,
  array_size_t num_groups,
  float min_score,
  enum RateConfig_Filter bad_filter
) {
  for (array_size_t group_id = 0; group_id < num_groups; ++group_id) {
    if (! RateConfig_GroupHasMinScore(results, group_id, min_score))
      continue;

    if (! RateConfig_GroupHasFanCount(results, group_id))
      continue;

    if (! RateConfig_GroupFilterBad(results, group_id, bad_filter))
      continue;

    RateConfig_PrintResultGroup(results, group_id);
    printf("\n");
  }
}

static void RateConfig_AddJsonResult(
  nx_json* array,
  array_of(ConfigWithData)* results,
  array_size_t group_id
) {
  ConfigWithData* last_result = NULL;
  nx_json* object = create_json_object(NULL, array);
  nx_json* files = create_json_array("files", object);

  for_each_array(ConfigWithData*, result, *results) {
    if (result->group_id != group_id)
      continue;

    create_json_string(NULL, files, result->file);
    last_result = result;
  }

  ConfigRating_ToJson(&last_result->rating, "rating", object);
}

static void RateConfig_PrintResultsJson(
  array_of(ConfigWithData)* results,
  array_size_t num_groups,
  float min_score,
  enum RateConfig_Filter bad_filter
) {
  nx_json root = {0};
  nx_json* array = create_json_array(NULL, &root);

  for (array_size_t group_id = 0; group_id < num_groups; ++group_id) {
    if (! RateConfig_GroupHasMinScore(results, group_id, min_score))
      continue;

    if (! RateConfig_GroupHasFanCount(results, group_id))
      continue;

    if (! RateConfig_GroupFilterBad(results, group_id, bad_filter))
      continue;

    RateConfig_AddJsonResult(array, results, group_id);
  }

  nxjson_write_to_fd(array, STDOUT_FILENO, 2);

#if STRICT_CLEANUP
  nx_json_free(array);
#endif
}

/*
 * Rate the configuration files given in the `files` array.
 *
 * Print result to stdout.
 */
static Error RateConfig_RateFiles(
  ConfigRating* config_rating,
  array_of(ConfigFile)* files,
  bool json,
  float min_score,
  enum RateConfig_Filter bad_filter
) {
  array_of(ConfigWithData) ratings;

  // Load model configuration and rate them
  ratings = RateConfig_RateConfigs(config_rating, files);

  // Sort ratings by priority
  RateConfig_SortResultByPriority(&ratings);

  // Sort ratings by score
  RateConfig_SortResultByScore(&ratings);

  // Set grouping
  array_size_t num_groups = RateConfig_GroupRatingsBySimilarConfig(&ratings);

  // Print results
  if (json)
    RateConfig_PrintResultsJson(&ratings, num_groups, min_score, bad_filter);
  else
    RateConfig_PrintResults(&ratings, num_groups, min_score, bad_filter);

  // Free
#if STRICT_CLEANUP
  for_each_array(ConfigWithData*, rating, ratings) {
    ConfigWithData_Free(rating);
  }
  Mem_Free(ratings.data);
#endif

  return err_success();
}

static inline void PrintFullHelpNotice(void) {
  printf(
    "Please run `nbfc rate-config --full-help` for a full explanation of how "
    "to interpret these results.\n");
}

/*
 * Rate many configuration files.
 *
 * Print result to stdout.
 */
static Error RateConfig_RateMany(
  ConfigRating* config_rating,
  array_of(ConfigFile)* files,
  bool json,
  float min_score,
  enum RateConfig_Filter bad_filter
) {
  Error e;

  // Rate configs
  e = RateConfig_RateFiles(config_rating, files, json, min_score, bad_filter);
  if (!json) {
    if (! RateConfig_Options.min_score_set) {
      printf(
        "Only configurations with the minimum recommended score of %.2f are shown.\n"
        "You can change this threshold by using -m|--min-score, but doing so can lead\n"
        "to unsafe configurations being displayed.\n\n", RATE_CONFIG_RECOMMENDED_MINIMUM_SCORE
      );
    }
    PrintFullHelpNotice();
  }

  return e;
}

/*
 * Rate all available configuration files.
 *
 * Print result to stdout.
 */
static Error RateConfig_RateAll(
  ConfigRating* config_rating,
  bool json,
  float min_score,
  enum RateConfig_Filter bad_filter
) {
  Error e;

  // Get all configuration files
  array_of(ConfigFile) all_configs = List_All_Configs();

  // Do the rating
  e = RateConfig_RateMany(config_rating, &all_configs, json, min_score, bad_filter);

  // Free
#if STRICT_CLEANUP
  ConfigFiles_Free(&all_configs);
#endif

  return e;
}

/*
 * Read configuration files from a file an rate them.
 *
 * Print result to stdout.
 */
static Error RateConfig_RateFromFile(
  ConfigRating* config_rating,
  const char* file,
  bool json,
  float min_score,
  enum RateConfig_Filter bad_filter
) {
  Error e;
  char* content;
  file_op_result res;
  array_of(ConfigFile) files;

  // Check for '-'
  if (! strcmp(file, "-"))
    file = "/dev/stdin";

  // Read the file
  res = slurp_file_dynamic(&content, file);
  if (! res.ok)
    return err_stdlib(file);

  // Allocate space
  files.size = 0;
  files.data = Mem_Calloc((str_count_newlines(content) + 1), sizeof(ConfigFile));

  // Populate files array with lines
  char* line = content;
  for (char* p = content; *p; ++p) {
    if (*p == '\n') {
      *p = '\0';

      if (strlen(line))
        files.data[files.size++].config_name = line;

      line = p + 1;
    }
  }

  if (strlen(line))
    files.data[files.size++].config_name = line;

  // Do the rating
  e = RateConfig_RateMany(config_rating, &files, json, min_score, bad_filter);

  // Free
#if STRICT_CLEANUP
  Mem_Free(content);
  Mem_Free(files.data);
#endif

  return e;
}

/*
 * Rate a single configuration file.
 *
 * Print result to stdout.
 */
static Error RateConfig_RateSingle(ConfigRating* config_rating, bool json, const char* file) {
  Error e;
  ConfigFile cfg_file;
  array_of(ConfigFile) configs;

  cfg_file.config_name = (char*) file;

  configs.size = 1;
  configs.data = &cfg_file;

  e = RateConfig_RateFiles(config_rating, &configs, json, 0.0, RateConfig_FilterAll);
  if (!json)
    PrintFullHelpNotice();

  return e;
}

/*
 * Print configuration rules to stdout.
 */
static int RateConfig_PrintRules(const char* rules_json, bool json) {
  Error e;
  ConfigRatingRules rules = {0};
  nx_json* js = NULL;

  e = ConfigRatingRules_FromJson(&rules, rules_json);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (json) {
    js = ConfigRatingRules_ToJson(&rules);
    nxjson_write_to_fd(js, STDOUT_FILENO, 2);
  }
  else {
    ConfigRatingRules_Print(&rules);
  }

#if STRICT_CLEANUP
  nx_json_free(js);
  ConfigRatingRules_Free(&rules);
#endif

  return NBFC_EXIT_SUCCESS;
}

static Error RateConfig_MakeAMLFilesArray(array_of(str)* out) {
  if (RateConfig_Options.dsdt_files_size) {
    out->data = RateConfig_Options.dsdt_files;
    out->size = RateConfig_Options.dsdt_files_size;
    return err_success();
  }
  else if (RateConfig_Options.dsdt_dir) {
    return AcpiAnalysis_GetAmlFiles(RateConfig_Options.dsdt_dir, out);
  }
  else {
    return AcpiAnalysis_GetAmlFiles(NULL, out);
  }
}

int RateConfig(void) {
  Error e;
  char* rules;
  ConfigRating config_rating = {0};
  array_of(str) dsdt_files = {0};

  // ==========================================================================
  // Check command line arguments
  // ==========================================================================

  if (RateConfig_Options.action == RateConfig_Action_None) {
    Log_Error("Missing configuration file");
    return NBFC_EXIT_CMDLINE;
  }

  // ==========================================================================
  // Print full help
  // ==========================================================================

  if (RateConfig_Options.action == RateConfig_Action_PrintFullHelp) {
    PrintFullHelp();
    return NBFC_EXIT_SUCCESS;
  }

  // ==========================================================================
  // Initialize curl (used for retrieving rating rules)
  // ==========================================================================

  CURLcode code = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (code != CURLE_OK) {
    Log_Error("curl_global_init() failed");
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Print configuration rules
  // ==========================================================================

  if (RateConfig_Options.action == RateConfig_Action_PrintRules) {
    rules = RateConfig_GetRules(RateConfig_Options.rules_file, RateConfig_Options.no_download);
    if (! rules)
      return NBFC_EXIT_FAILURE;
    int ret = RateConfig_PrintRules(rules, RateConfig_Options.json);
#if STRICT_CLEANUP
    Mem_Free(rules);
#endif
    return ret;
  }

  // ==========================================================================
  // Check if AML files are readable
  // ==========================================================================

  if (! RateConfig_Options.dsdt_files_size && ! RateConfig_Options.dsdt_dir) {
    check_root();
  }

  for (size_t i = 0; i < RateConfig_Options.dsdt_files_size; ++i) {
    if (! file_is_readable(RateConfig_Options.dsdt_files[i])) {
      Log_Error("%s: %s", RateConfig_Options.dsdt_files[i], strerror(errno));
      return NBFC_EXIT_FAILURE;
    }
  }

  e = RateConfig_MakeAMLFilesArray(&dsdt_files);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Check if needed programs are installed
  // ==========================================================================

  e = AcpiAnalysis_IsAcpiExecInstalled();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Initialize ConfigRating
  // ==========================================================================

  rules = RateConfig_GetRules(RateConfig_Options.rules_file, RateConfig_Options.no_download);
  if (! rules)
    return NBFC_EXIT_FAILURE;

  e = ConfigRating_Init(&config_rating, &dsdt_files, rules);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Add unverified EC registers
  // ==========================================================================

  if (RateConfig_Options.unverified)
    AcpiAnalysis_AddUnverifiedEmbeddedControllerRegions(&config_rating.acpi_info);

  // ==========================================================================
  // Call desired function
  // ==========================================================================

  if (RateConfig_Options.action == RateConfig_Action_RateAll) {
    e = RateConfig_RateAll(
        &config_rating,
        RateConfig_Options.json,
        RateConfig_Options.min_score,
        RateConfig_Options.filter);
  }
  else if (RateConfig_Options.action == RateConfig_Action_RateFromFile) {
    e = RateConfig_RateFromFile(
        &config_rating,
        RateConfig_Options.input_file,
        RateConfig_Options.json,
        RateConfig_Options.min_score,
        RateConfig_Options.filter);
  }
  else {
    e = RateConfig_RateSingle(
        &config_rating,
        RateConfig_Options.json,
        RateConfig_Options.file);
  }

#if STRICT_CLEANUP
  ConfigRating_Free(&config_rating);
  Mem_Free(rules);
  curl_global_cleanup();
#endif

  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }
  else
    return NBFC_EXIT_SUCCESS;
}
