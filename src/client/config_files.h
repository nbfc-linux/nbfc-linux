#ifndef NBFC_CONFIG_FILES_H_
#define NBFC_CONFIG_FILES_H_

#include "../macros.h"

#define RecommendedConfigMatchThreshold 0.7f

struct ConfigFile {
  char* config_name;
  float diff;
};
typedef struct ConfigFile ConfigFile;
declare_array_of(ConfigFile);

array_of(ConfigFile) List_All_Configs();
array_of(ConfigFile) List_Recommended_Configs();

char* Get_Supported_Config(array_of(ConfigFile)*, const char*);
bool  Contains_Config(array_of(ConfigFile)*, const char*);
void  ConfigFiles_Free(array_of(ConfigFile)*);

int ConfigFile_CompareByName(const void*, const void*);
int ConfigFile_CompareByDiff(const void*, const void*);

#endif
