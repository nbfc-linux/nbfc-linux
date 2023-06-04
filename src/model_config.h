#ifndef NBFC_MODEL_CONFIG_H_
#define NBFC_MODEL_CONFIG_H_

#if defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)

#define NBFC_PACKED_ENUM  __attribute__((packed))

enum NBFC_PACKED_ENUM RegisterWriteMode_ {
  RegisterWriteMode_Set,
  RegisterWriteMode_And,
  RegisterWriteMode_Or,
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

enum NBFC_PACKED_ENUM Boolean_ {
  Boolean_False = 0,
  Boolean_True  = 1,
  Boolean_Unset,
};

typedef enum RegisterWriteMode_       RegisterWriteMode;
typedef enum RegisterWriteOccasion_   RegisterWriteOccasion;
typedef enum OverrideTargetOperation_ OverrideTargetOperation;
typedef enum EmbeddedControllerType_  EmbeddedControllerType;
typedef enum Boolean_                 Boolean;

#else /* no packed enums */

typedef char                          RegisterWriteMode;
typedef char                          RegisterWriteOccasion;
typedef char                          OverrideTargetOperation;
typedef char                          EmbeddedControllerType;
typedef char                          Boolean;

#endif /* packed enums */

#include "macros.h"
#include "nxjson.h"
#include "error.h"
#include "generated/model_config.generated.h"

EmbeddedControllerType EmbeddedControllerType_FromString(const char*);
const char*            EmbeddedControllerType_ToString(EmbeddedControllerType);
Error* ModelConfig_Validate(ModelConfig*);
Error* ModelConfig_FromFile(ModelConfig*, const char*);
void   ModelConfig_Free(ModelConfig*);

#endif
