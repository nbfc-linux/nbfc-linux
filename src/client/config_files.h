#ifndef CONFIG_FILES_H_
#define CONFIG_FILES_H_

#include "../macros.h"

#define RecommendedConfigMatchThreshold 0.7f

struct ConfigFile {
  char *config_name;
  float diff;
};
typedef struct ConfigFile ConfigFile;
declare_array_of(ConfigFile);

array_of(ConfigFile) get_configs();
array_of(ConfigFile) recommended_configs();

int compare_config_by_name(const void*, const void*);
int compare_config_by_diff(const void*, const void*);

char* get_supported_model(array_of(ConfigFile)*, const char*);

#endif
