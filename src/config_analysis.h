#ifndef NBFC_CONFIG_ANALYSIS_H_
#define NBFC_CONFIG_ANALYSIS_H_

#include "error.h"
#include "macros.h"
#include "model_config.h"

#include <stdint.h>

#define CONFIG_ANALYSIS_MAX_REGISTERS  256
#define CONFIG_ANALYSIS_MAX_METHODS    64
#define CONFIG_ANALYSIS_MAX_METHOD_LEN 128

/*
 * Specifies the category of a register, e.g. whether it is a fan register or
 * a miscellaneous register used for register-write configurations.
 */
enum NBFC_PACKED_ENUM RegisterType {
  RegisterType_FanReadRegister,
  RegisterType_FanWriteRegister,
  RegisterType_RegisterWriteConfigurationRegister
};

typedef struct RegisterAnalysis RegisterAnalysis;
struct RegisterAnalysis {
  uint8_t register_;
  enum RegisterType type;
  bool word;
};

typedef struct ConfigAnalysis ConfigAnalysis;
struct ConfigAnalysis {
  RegisterAnalysis registers[CONFIG_ANALYSIS_MAX_REGISTERS];
  size_t registers_size;
  char methods[CONFIG_ANALYSIS_MAX_METHODS][CONFIG_ANALYSIS_MAX_METHOD_LEN + 1];
  size_t methods_size;
};

Error ConfigAnalysis_AnalyzeModelConfig(ModelConfig*, ConfigAnalysis**);

#endif
