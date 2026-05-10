#include "../xml2json.h"
#include "../model_config_to_json.h"
#include "../nxjson_write.h"

#include "client_global.h"

const struct cli99_Option xml2json_options[] = {
  cli99_Options_Include(&main_options),
  {"file", Option_Xml2Json_File, cli99_NormalPositional},
  cli99_Options_End()
};

struct {
  const char* file;
} Xml2Json_Options = {
  NULL,
};

int Xml2Json(void) {
  Error e;
  nx_json* js;
  Trace trace;
  ModelConfig model_config;

  if (! Xml2Json_Options.file) {
    Log_Error("Missing configuration file");
    return NBFC_EXIT_CMDLINE;
  }

  // Convert XML file to JSON
  e = Xml2Json_ConvertFile(Xml2Json_Options.file, &js);
  if (e) {
    Log_Error("%s: %s", Xml2Json_Options.file, err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // Load generated JSON into ModelConfig
  e = ModelConfig_FromJson(&model_config, js);
  if (e) {
    Log_Error("%s: %s", Xml2Json_Options.file, err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // Remove empty arrays and strings
  ModelConfig_RemoveEmptyArrays(&model_config);
  ModelConfig_RemoveEmptyStrings(&model_config);

  // Validate it
  Trace_Init(&trace);
  Trace_Push(&trace, "%s", Xml2Json_Options.file);
  e = ModelConfig_Validate(&trace, &model_config);
  if (e) {
    Log_Error("%s: %s", Xml2Json_Options.file, err_print_all(e));
    return NBFC_EXIT_FAILURE;
  }

  // Set LegacyTemperatureThresholdsBehaviour to true
  model_config.LegacyTemperatureThresholdsBehaviour = true;
  ModelConfig_Set_LegacyTemperatureThresholdsBehaviour(&model_config);

  // Convert ModelConfig back to JSON and print it
  nx_json root = {0};
  ModelConfig_ToJson(&model_config, &root, NULL);
  nxjson_write_to_fd(root.val.children.first, STDOUT_FILENO, 2);
  return NBFC_EXIT_SUCCESS;
}
