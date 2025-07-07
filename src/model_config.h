#ifndef NBFC_MODEL_CONFIG_H_
#define NBFC_MODEL_CONFIG_H_

#include "config.h"
#include "macros.h"
#include "nxjson.h"
#include "error.h"
#include "trace.h"

#if defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)

#define NBFC_PACKED_ENUM  __attribute__((packed))

enum NBFC_PACKED_ENUM RegisterWriteMode_ {
  RegisterWriteMode_Set,
  RegisterWriteMode_And,
  RegisterWriteMode_Or,
  RegisterWriteMode_Call,
  RegisterWriteMode_Unset,
};

enum NBFC_PACKED_ENUM RegisterWriteOccasion_ {
  RegisterWriteOccasion_OnWriteFanSpeed,
  RegisterWriteOccasion_OnInitialization,
  RegisterWriteOccasion_Unset,
};

enum NBFC_PACKED_ENUM OverrideTargetOperation_ {
  OverrideTargetOperation_Read      = 0x1,
  OverrideTargetOperation_Write     = 0x2,
  OverrideTargetOperation_ReadWrite = 0x3,
  OverrideTargetOperation_Unset,
};

enum NBFC_PACKED_ENUM EmbeddedControllerType_ {
  EmbeddedControllerType_ECSysLinux,
  EmbeddedControllerType_ECSysLinuxACPI,
  EmbeddedControllerType_ECLinux,
  EmbeddedControllerType_ECDummy,
  EmbeddedControllerType_Unset,
};

enum NBFC_PACKED_ENUM TemperatureAlgorithmType_ {
  TemperatureAlgorithmType_Average,
  TemperatureAlgorithmType_Min,
  TemperatureAlgorithmType_Max,
  TemperatureAlgorithmType_Unset,
};

typedef enum RegisterWriteMode_        RegisterWriteMode;
typedef enum RegisterWriteOccasion_    RegisterWriteOccasion;
typedef enum OverrideTargetOperation_  OverrideTargetOperation;
typedef enum EmbeddedControllerType_   EmbeddedControllerType;
typedef enum TemperatureAlgorithmType_ TemperatureAlgorithmType;

#else /* no packed enums */

typedef char                          RegisterWriteMode;
typedef char                          RegisterWriteOccasion;
typedef char                          OverrideTargetOperation;
typedef char                          EmbeddedControllerType;
typedef char                          TemperatureAlgorithmType;

#endif /* packed enums */

typedef const char* str;
declare_array_of(str);

#include "generated/model_config.generated.h"

EmbeddedControllerType    EmbeddedControllerType_FromString(const char*);
const char*               EmbeddedControllerType_ToString(EmbeddedControllerType);
TemperatureAlgorithmType  TemperatureAlgorithmType_FromString(const char*);
const char*               TemperatureAlgorithmType_ToString(TemperatureAlgorithmType);

Error* ModelConfig_Validate(Trace*, ModelConfig*);
Error* ModelConfig_FromFile(ModelConfig*, const char*);
Error* ModelConfig_FindAndLoad(ModelConfig*, char*, const char*);
void   ModelConfig_Free(ModelConfig*);

#endif
