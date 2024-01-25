#include <string.h>
#include <locale.h>

#include "log.h"
#include "nbfc.h"
#include "error.c"
#include "memory.c"
#include "nxjson.c"
#include "model_config.c"
#include "program_name.c"

int main(int argc, const char** argv) {
  Program_Name_Set(argv[0]);

  setlocale(LC_NUMERIC, "C"); // for json floats

  if (argc < 2) {
    Log_Error("Missing file\n");
    return NBFC_EXIT_CMDLINE;
  }

  const char* file = argv[1];
  char path[PATH_MAX];
  ModelConfig model_config = {0};

  if (strchr(file, '/')) {
    snprintf(path, PATH_MAX, "%s", file);
  }
  else {
    snprintf(path, PATH_MAX, "%s/%s.json", NBFC_MODEL_CONFIGS_DIR, file);
  }

  Error* e = ModelConfig_FromFile(&model_config, path);
  e_die();

  e = ModelConfig_Validate(&model_config);
  e_die();

  return 0;
}
