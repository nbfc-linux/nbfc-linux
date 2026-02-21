#include "../nbfc.h"
#include "../error.h"
#include "../memory.h"
#include "../acpi_analysis.h"
#include "../config_rating.h"
#include "../file_utils.h"
#include "../model_config_utils.h"
#include "../nxjson_utils.h"

#include <string.h> // memset
#include <linux/limits.h>

#include "curl_utils.h"
#include "config_files.h"
#include "check_root.h"
#include "client_global.h"

#define RATE_CONFIG_RECOMMENDED_MINIMUM_SCORE 9.0

#define RATE_CONFIG_RULES_JSON_URL \
  "https://raw.githubusercontent.com/nbfc-linux/nbfc-linux/main/endpoints/config_rating_rules_v1.json"

const struct cli99_Option rate_config_options[] = {
  cli99_Options_Include(&main_options),
  {"-d|--dsdt",        Option_Rate_Config_DSDT_File,   cli99_RequiredArgument},
  {"-a|--all",         Option_Rate_Config_All,         cli99_NoArgument      },
  {"-H|--full-help",   Option_Rate_Config_Full_Help,   cli99_NoArgument      },
  {"-j|--json",        Option_Rate_Config_Json,        cli99_NoArgument      },
  {"-m|--min-score",   Option_Rate_Config_Min_Score,   cli99_RequiredArgument},
  {"-n|--no-download", Option_Rate_Config_No_Download, cli99_NoArgument      },
  {"-r|--rules",       Option_Rate_Config_Rules,       cli99_RequiredArgument},
  {"--print-rules",    Option_Rate_Config_Print_Rules, cli99_NoArgument      },
  {"file",             Option_Rate_Config_File,        cli99_NormalPositional},
  cli99_Options_End()
};

struct {
  bool        all;
  bool        full_help;
  bool        json;
  bool        print_rules;
  bool        no_download;
  bool        min_score_set;
  float       min_score;
  const char* file;
  const char* dsdt_file;
  const char* rules_file;
} Rate_Config_Options = {
  false,
  false,
  false,
  false,
  false,
  false,
  RATE_CONFIG_RECOMMENDED_MINIMUM_SCORE,
  NULL,
  ACPI_ANALYSIS_ACPI_DSDT,
  NULL,
};

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
    Log_Error("Download failed: %s (server returned HTTP %ld)\n",
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
    if (slurp_file_dynamic(&out, rules_file) == -1) {
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

static void PrintFullHelp() {
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
    Trace_Push(&trace, path);
    LogLevel old = Log_LogLevel;
    Log_LogLevel = LogLevel_Quiet;
    e = ModelConfig_Validate(&trace, &config_with_data->model_config);
    Log_LogLevel = old;
    if (e) {
      Log_Warn("%s", err_print_all(e));
      ModelConfig_Free(&config_with_data->model_config);
      continue;
    }

    // Set the filename
    config_with_data->file = Mem_Strdup(file->config_name);

    // Rate the config
    ConfigRating_RateModelConfig(
      config_rating,
      &config_with_data->model_config,
      &config_with_data->rating
    );

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

      if (ModelConfig_IsSimilar(&configs->data[i].model_config, &configs->data[j].model_config))
        configs->data[j].group_id = next_group;
    }

    ++next_group;
  }

  return next_group;
}

static void RateConfig_SortResultByScore(array_of(ConfigWithData)* result) {
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

static void RateConfig_PrintResultGroup(
  array_of(ConfigWithData)* results,
  array_size_t group_id
) {
  ConfigWithData* last_result = NULL;

  for_each_array(ConfigWithData*, result, *results) {
    if (result->group_id != group_id)
      continue;

    printf("%s\n", result->file);
    last_result = result;
  }

  ConfigRating_RatingPrint(&last_result->rating);
}

static void RateConfig_PrintResults(
  array_of(ConfigWithData)* results,
  array_size_t num_groups,
  float min_score
) {
  for (array_size_t group_id = 0; group_id < num_groups; ++group_id) {
    if (! RateConfig_GroupHasMinScore(results, group_id, min_score))
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
  float min_score
) {
  nx_json root = {0};
  nx_json* array = create_json_array(NULL, &root);

  for (array_size_t group_id = 0; group_id < num_groups; ++group_id) {
    if (! RateConfig_GroupHasMinScore(results, group_id, min_score))
      continue;

    RateConfig_AddJsonResult(array, results, group_id);
  }

  nxjson_write_to_fd(array, STDOUT_FILENO);

  nx_json_free(array);
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
  float min_score
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
    RateConfig_PrintResultsJson(&ratings, num_groups, min_score);
  else
    RateConfig_PrintResults(&ratings, num_groups, min_score);

  // Free
  for_each_array(ConfigWithData*, rating, ratings) {
    ConfigWithData_Free(rating);
  }
  Mem_Free(ratings.data);

  return err_success();
}

static inline void PrintFullHelpNotice() {
  printf(
    "Please run `nbfc rate-config --full-help` for a full explanation of how "
    "to interpret these results.\n");
}

/*
 * Rate all available configuration files.
 *
 * Print result to stdout.
 */
static Error RateConfig_RateAll(ConfigRating* config_rating, bool json, float min_score) {
  Error e;

  // Get all configuration files
  array_of(ConfigFile) all_configs = List_All_Configs();

  // Rate configs
  e = RateConfig_RateFiles(config_rating, &all_configs, json, min_score);
  if (!json) {
    if (! Rate_Config_Options.min_score_set) {
      printf(
        "Only configurations with the minimum recommended score of %.2f are shown.\n"
        "You can change this threshold by using -m|--min-score, but doing so can lead\n"
        "to unsafe configurations being displayed.\n\n", RATE_CONFIG_RECOMMENDED_MINIMUM_SCORE
      );
    }
    PrintFullHelpNotice();
  }

  // Free
  ConfigFiles_Free(&all_configs);

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

  e = RateConfig_RateFiles(config_rating, &configs, json, 0.0);
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

  e = ConfigRatingRules_FromJson(&rules, rules_json);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  if (json) {
    nx_json* json = ConfigRatingRules_ToJson(&rules);
    nxjson_write_to_fd(json, STDOUT_FILENO);
    nx_json_free(json);
  }
  else {
    ConfigRatingRules_Print(&rules);
  }

  ConfigRatingRules_Free(&rules);

  return NBFC_EXIT_SUCCESS;
}

int RateConfig() {
  Error e;
  char* rules;
  ConfigRating config_rating = {0};

  if (Rate_Config_Options.full_help) {
    PrintFullHelp();
    return NBFC_EXIT_SUCCESS;
  }

  // ==========================================================================
  // Check command line arguments
  // ==========================================================================

  if (! Rate_Config_Options.print_rules && ! Rate_Config_Options.all && ! Rate_Config_Options.file) {
    Log_Error("Missing configuration file");
    return NBFC_EXIT_CMDLINE;
  }

  if (Rate_Config_Options.all && Rate_Config_Options.file) {
    Log_Error("-a|--all cannot be used together with a filename");
    return NBFC_EXIT_CMDLINE;
  }

  if (Rate_Config_Options.print_rules && Rate_Config_Options.all) {
    Log_Error("--print-rules cannot be used together with -a");
    return NBFC_EXIT_CMDLINE;
  }

  if (Rate_Config_Options.print_rules && Rate_Config_Options.file) {
    Log_Error("--print-rules cannot be used together with a filename");
    return NBFC_EXIT_CMDLINE;
  }

  // ==========================================================================
  // Print configuration rules
  // ==========================================================================

  if (Rate_Config_Options.print_rules) {
    rules = RateConfig_GetRules(Rate_Config_Options.rules_file, Rate_Config_Options.no_download);
    if (! rules)
      return NBFC_EXIT_FAILURE;
    return RateConfig_PrintRules(rules, Rate_Config_Options.json);
  }

  // ==========================================================================
  // Check if DSDT file exists and is readable
  // ==========================================================================

  if (! file_exists(Rate_Config_Options.dsdt_file)) {
    Log_Error("%s: %s", Rate_Config_Options.dsdt_file, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }

  if (! file_is_readable(Rate_Config_Options.dsdt_file)) {
    Log_Error("%s: %s (do you need root priviledges?)", Rate_Config_Options.dsdt_file, strerror(errno));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Check if needed programs are installed
  // ==========================================================================

  e = Acpi_Analysis_Is_AcpiExec_Installed();
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Initialize ConfigRating
  // ==========================================================================

  rules = RateConfig_GetRules(Rate_Config_Options.rules_file, Rate_Config_Options.no_download);
  if (! rules)
    return NBFC_EXIT_FAILURE;

  e = ConfigRating_Init(&config_rating, Rate_Config_Options.dsdt_file, rules);
  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // ==========================================================================
  // Call desired function
  // ==========================================================================

  if (Rate_Config_Options.all)
    e = RateConfig_RateAll(&config_rating, Rate_Config_Options.json, Rate_Config_Options.min_score);
  else
    e = RateConfig_RateSingle(&config_rating, Rate_Config_Options.json, Rate_Config_Options.file);

  ConfigRating_Free(&config_rating);
  Mem_Free(rules);

  if (e) {
    Log_Error("%s", err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }
  else
    return NBFC_EXIT_SUCCESS;
}
