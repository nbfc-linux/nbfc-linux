#ifndef NBFC_MODEL_CONFIG_TO_JSON_H_
#define NBFC_MODEL_CONFIG_TO_JSON_H_

#include "model_config.h"
#include "nxjson.h"

void ModelConfig_ToJson(const ModelConfig*, nx_json*, const char*);
void ModelConfig_RemoveEmptyArrays(ModelConfig*);
void ModelConfig_RemoveEmptyStrings(ModelConfig*);

#endif
