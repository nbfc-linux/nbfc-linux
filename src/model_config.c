#include "model_config.h"

#include "nbfc.h"
#include "log.h"
#include "macros.h"
#include "memory.h"
#include "stack_memory.h"
#include "nxjson_utils.h"

#include <assert.h>  // assert
#include <string.h>  // strcmp
#include <unistd.h>  // access, F_OK
#include <stdbool.h> // bool
#include <limits.h>  // INT_MIN, SHRT_MIN
#include <math.h>    // NAN
#include <linux/limits.h>

static inline Error* bool_FromJson(bool* out, const nx_json* node) {
  if (node->type == NX_JSON_BOOL) {
    *out = node->val.u;
    return err_success();
  }
  return err_string(0, "Not a bool");
}

static inline Error* int_FromJson(int* out, const nx_json* node) {
  if (node->type == NX_JSON_INTEGER) {
    *out = node->val.i;
    return err_success();
  }
  return err_string(0, "Not an integer");
}

static inline Error* int8_t_FromJson(int8_t* out, const nx_json* node) {
  int val; // NOLINT
  Error* e = int_FromJson(&val, node);
  e_check();
  if (val < INT8_MIN || val > INT8_MAX)
    return err_stringf(0, "Value not in range (%d - %d): %d", INT8_MIN, INT8_MAX, val);
  *out = val;
  return err_success();
}

static inline Error* uint8_t_FromJson(uint8_t* out, const nx_json* node) {
  int val; // NOLINT
  Error* e = int_FromJson(&val, node);
  e_check();
  if (val < 0 || val > UINT8_MAX)
    return err_stringf(0, "Value not in range (%d - %d): %d", 0, UINT8_MAX, val);
  *out = val;
  return err_success();
}

static inline Error* int16_t_FromJson(int16_t* out, const nx_json* node) {
  int val; // NOLINT
  Error* e = int_FromJson(&val, node);
  e_check();
  if (val < INT16_MIN || val > INT16_MAX)
    return err_stringf(0, "Value not in range (%d - %d): %d", INT16_MIN, INT16_MAX, val);
  *out = val;
  return err_success();
}

static inline Error* uint16_t_FromJson(uint16_t* out, const nx_json* node) {
  int val; // NOLINT
  Error* e = int_FromJson(&val, node);
  e_check();
  if (val < 0 || val > UINT16_MAX)
    return err_stringf(0, "Value not in range (%d - %d): %d", 0, UINT16_MAX, val);
  *out = val;
  return err_success();
}

static inline Error* double_FromJson(double* out, const nx_json* node) {
  if (node->type == NX_JSON_INTEGER) {
    *out = node->val.i;
    return err_success();
  }
  if (node->type == NX_JSON_DOUBLE) {
    *out = node->val.dbl;
    return err_success();
  }
  return err_string(0, "Not a double");
}

static inline Error* float_FromJson(float* out, const nx_json* json) {
  double d = 0;
  Error* e = double_FromJson(&d, json);
  if (! e)
    *out = d;
  return e;
}

static inline Error* str_FromJson(const char** out, const nx_json* json) {
  Error* e = nx_json_get_str(out, json);
  e_check();
  *out = Mem_Strdup(*out);
  return err_success();
}

static Error* RegisterWriteMode_FromJson(RegisterWriteMode* out, const nx_json* json) {
  const char* s; // NOLINT
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  else if (!strcmp(s, "Set"))  *out = RegisterWriteMode_Set;
  else if (!strcmp(s, "And"))  *out = RegisterWriteMode_And;
  else if (!strcmp(s, "Or"))   *out = RegisterWriteMode_Or;
  else if (!strcmp(s, "Call")) *out = RegisterWriteMode_Call;
  else return err_stringf(0, "Invalid value for %s: %s", "RegisterWriteMode", s);
  return e;
}

static Error* RegisterWriteOccasion_FromJson(RegisterWriteOccasion* out, const nx_json* json) {
  const char* s; // NOLINT
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  else if (!strcmp(s, "OnWriteFanSpeed"))  *out = RegisterWriteOccasion_OnWriteFanSpeed;
  else if (!strcmp(s, "OnInitialization")) *out = RegisterWriteOccasion_OnInitialization;
  else return err_stringf(0, "Invalid value for %s: %s", "RegisterWriteOccasion", s);
  return e;
}

static Error* OverrideTargetOperation_FromJson(OverrideTargetOperation* out, const nx_json* json) {
  const char* s; // NOLINT
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  else if (!strcmp(s, "Read"))       *out = OverrideTargetOperation_Read;
  else if (!strcmp(s, "Write"))      *out = OverrideTargetOperation_Write;
  else if (!strcmp(s, "ReadWrite"))  *out = OverrideTargetOperation_ReadWrite;
  else return err_stringf(0, "Invalid value for %s: %s", "OverrideTargetOperation", s);
  return e;
}

TemperatureAlgorithmType TemperatureAlgorithmType_FromString(const char* s) {
  if (!strcmp(s, "Average")) return TemperatureAlgorithmType_Average;
  if (!strcmp(s, "Min"))     return TemperatureAlgorithmType_Min;
  if (!strcmp(s, "Max"))     return TemperatureAlgorithmType_Max;
  return TemperatureAlgorithmType_Unset;
}

static Error* TemperatureAlgorithmType_FromJson(TemperatureAlgorithmType* out, const nx_json* json) {
  const char* s; // NOLINT
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  TemperatureAlgorithmType a = TemperatureAlgorithmType_FromString(s);
  if (a == TemperatureAlgorithmType_Unset)
    return err_stringf(0, "Invalid value for %s: %s", "TemperatureAlgorithmType", s);
  *out = a;
  return e;
}

static Error* EmbeddedControllerType_FromJson(EmbeddedControllerType* out, const nx_json* json) {
  const char* s; // NOLINT
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  EmbeddedControllerType t = EmbeddedControllerType_FromString(s);
  if (t == EmbeddedControllerType_Unset)
    return err_stringf(0, "Invalid value for %s: %s", "EmbeddedControllerType", s);
  *out = t;
  return e;
}

EmbeddedControllerType EmbeddedControllerType_FromString(const char* s) {
  if (!strcmp(s, "ec_sys")) {
#if ENABLE_EC_SYS
    return EmbeddedControllerType_ECSysLinux;
#else
ec_sys_disabled:
    Log_Error("EmbeddedControllerType 'ec_sys' has been disabled at compile time.\n");
    return EmbeddedControllerType_Unset;
#endif
  }

  if (!strcmp(s, "acpi_ec")) {
#if ENABLE_EC_ACPI
    return EmbeddedControllerType_ECSysLinuxACPI;
#else
ec_acpi_disabled:
    Log_Error("EmbeddedControllerType 'acpi_ec' has been disabled at compile time.\n");
    return EmbeddedControllerType_Unset;
#endif
  }

  if (!strcmp(s, "dev_port")) {
#if ENABLE_EC_DEV_PORT
    return EmbeddedControllerType_ECLinux;
#else
ec_dev_port_disabled:
    Log_Error("EmbeddedControllerType 'dev_port' has been disabled at compile time.\n");
    return EmbeddedControllerType_Unset;
#endif
  }

  if (!strcmp(s, "dummy")) {
#if ENABLE_EC_DUMMY
    return EmbeddedControllerType_ECDummy;
#else
    Log_Error("EmbeddedControllerType 'dummy' has been disabled at compile time.\n");
#endif
  }

  // for older versions of nbfc-linux:
  if (!strcmp(s, "ec_sys_linux")) {
#if ENABLE_EC_SYS
    return EmbeddedControllerType_ECSysLinux;
#else
    goto ec_sys_disabled;
#endif
  }

  if (!strcmp(s, "ec_acpi")) {
#if ENABLE_EC_ACPI
    return EmbeddedControllerType_ECSysLinuxACPI;
#else
    goto ec_acpi_disabled;
#endif
  }

  if (!strcmp(s, "ec_linux")) {
#if ENABLE_EC_DEV_PORT
    return EmbeddedControllerType_ECLinux;
#else
    goto ec_dev_port_disabled;
#endif
  }

  return EmbeddedControllerType_Unset;
}

const char* EmbeddedControllerType_ToString(EmbeddedControllerType t) {
  switch (t) {
  case EmbeddedControllerType_ECSysLinux:     return "ec_sys";
  case EmbeddedControllerType_ECSysLinuxACPI: return "acpi_ec";
  case EmbeddedControllerType_ECLinux:        return "dev_port";
  case EmbeddedControllerType_ECDummy:        return "dummy";
  default: assert(!"Invalid value for EmbeddedControllerType");
  }
  return NULL;
}

const char* TemperatureAlgorithmType_ToString(TemperatureAlgorithmType t) {
  switch (t) {
  case TemperatureAlgorithmType_Average: return "Average";
  case TemperatureAlgorithmType_Min:     return "Min";
  case TemperatureAlgorithmType_Max:     return "Max";
  default: assert(!"Invalid value for TemperatureAlgorithmType");
  }
  return NULL;
}

typedef Error* (FromJson_Callback)(void*, const nx_json*);

static Error* array_of_FromJson(FromJson_Callback callback, void** v_data, ssize_t* v_size, ssize_t size, const nx_json* json) {
  Error* e = nx_json_get_array(json);
  e_check();

  *v_size = 0;

  // Mem_Malloc(0) returns non-NULL value, so we check for empty array here.
  if (! json->val.children.length) {
    *v_data = NULL;
    return err_success();
  }

  *v_data = Mem_Malloc(json->val.children.length * size);
  nx_json_for_each(child, json) {
    e = callback(((char*) *v_data) + size * *v_size, child);
    e_check();
    *v_size = *v_size + 1;
  }
  return err_success();
}

#define define_array_of_T_FromJson(T) \
static inline Error* array_of_##T##_FromJson(array_of(T)* v, const nx_json *json) { \
  return array_of_FromJson((FromJson_Callback*) T ## _FromJson, (void**) &v->data, &v->size, sizeof(T), json); \
}

define_array_of_T_FromJson(str)
define_array_of_T_FromJson(float)
define_array_of_T_FromJson(TemperatureThreshold)
define_array_of_T_FromJson(FanConfiguration)
define_array_of_T_FromJson(FanSpeedPercentageOverride)
define_array_of_T_FromJson(RegisterWriteConfiguration)
define_array_of_T_FromJson(FanInfo)
define_array_of_T_FromJson(FanTemperatureSourceConfig)

// ============================================================================
// Default temperature thresholds
// ============================================================================

static TemperatureThreshold _Config_DefaultTemperatureThresholds[] = {
  {60,  0,   0, 255},
  {63, 48,  10, 255},
  {66, 55,  20, 255},
  {68, 59,  50, 255},
  {71, 63,  70, 255},
  {75, 67, 100, 255},
};

static array_of(TemperatureThreshold) Config_DefaultTemperatureThresholds = {
  _Config_DefaultTemperatureThresholds,
  ARRAY_SIZE(_Config_DefaultTemperatureThresholds)
};

static TemperatureThreshold _Config_DefaultLegacyTemperatureThresholds[] = {
  {0,   0,   0, 255},
  {60, 48,  10, 255},
  {63, 55,  20, 255},
  {66, 59,  50, 255},
  {68, 63,  70, 255},
  {71, 67, 100, 255},
};

static array_of(TemperatureThreshold) Config_DefaultLegacyTemperatureThresholds = {
  _Config_DefaultLegacyTemperatureThresholds,
  ARRAY_SIZE(_Config_DefaultLegacyTemperatureThresholds)
};

static void copy_array_of_TemperatureThreshold(
  array_of(TemperatureThreshold)* dest,
  array_of(TemperatureThreshold)* src) {
  dest->size = src->size;
  dest->data = Mem_Malloc(src->size * sizeof(TemperatureThreshold));
  memcpy(dest->data, src->data, src->size * sizeof(TemperatureThreshold));
}

static array_of(FanSpeedPercentageOverride) Config_DefaultFanSpeedPercentageOverrides = {
  NULL,
  0
};

#include "generated/model_config.generated.c"

void ModelConfig_Free(ModelConfig* c) {
  Mem_Free((char*) c->NotebookModel);
  Mem_Free((char*) c->Author);

  for_each_array(FanConfiguration*, f, c->FanConfigurations) {
    Mem_Free((char*) f->FanDisplayName);
    Mem_Free((char*) f->ReadAcpiMethod);
    Mem_Free((char*) f->WriteAcpiMethod);
    Mem_Free((char*) f->ResetAcpiMethod);
    Mem_Free(f->TemperatureThresholds.data);
    Mem_Free(f->FanSpeedPercentageOverrides.data);
  }

  Mem_Free(c->FanConfigurations.data);

  for_each_array(RegisterWriteConfiguration*, r, c->RegisterWriteConfigurations) {
    Mem_Free((char*) r->AcpiMethod);
    Mem_Free((char*) r->ResetAcpiMethod);
    Mem_Free((char*) r->Description);
  }

  Mem_Free(c->RegisterWriteConfigurations.data);

  memset(c, 0, sizeof(*c));
}

// ============================================================================
// Validation code
// ============================================================================
//
// Calls *_ValidateFields on each structure and does some validations
// that cannot be auto-generated.


Error* TemperatureThresholds_Validate(
  Trace* trace,
  array_of(TemperatureThreshold)* TemperatureThresholds,
  int CriticalTemperature
)
{
  Error* e;
  bool has_0_FanSpeed   = false;
  bool has_100_FanSpeed = false;

  for_each_array(TemperatureThreshold*, t, *TemperatureThresholds) {
    Trace_Push(trace, "TemperatureThresholds[%d]", PTR_DIFF(t, TemperatureThresholds->data));

    e = TemperatureThreshold_ValidateFields(t);
    e_check();

    has_0_FanSpeed   |= (t->FanSpeed == 0);
    has_100_FanSpeed |= (t->FanSpeed == 100);

    if (t->UpThreshold < t->DownThreshold) {
      e = err_string(0, "UpThreshold cannot be less than DownThreshold");
      return e;
    }

    if (t->UpThreshold > CriticalTemperature) {
      Log_Warn("%s: UpThreshold cannot be greater than CriticalTemperature\n", trace->buf);
    }

    for_each_array(TemperatureThreshold*, t1, *TemperatureThresholds) {
      if (t != t1 && t->UpThreshold == t1->UpThreshold) {
        e = err_string(0, "Duplicate UpThreshold");
        return e;
      }
    }

    Trace_Pop(trace);
  }

  if (! has_0_FanSpeed)
    Log_Warn("%s: No threshold with FanSpeed == %d found\n", trace->buf, 0);

  if (! has_100_FanSpeed)
    Log_Warn("%s: No threshold with FanSpeed == %d found\n", trace->buf, 100);

  return err_success();
}

static Error* RegisterWriteConfiguration_Validate(const RegisterWriteConfiguration* r) {
  const bool AcpiMethod                  = RegisterWriteConfiguration_IsSet_AcpiMethod(r);
  const bool ResetAcpiMethod             = RegisterWriteConfiguration_IsSet_ResetAcpiMethod(r);
  const bool Register                    = RegisterWriteConfiguration_IsSet_Register(r);
  const bool Value                       = RegisterWriteConfiguration_IsSet_Value(r);
  const bool ResetValue                  = RegisterWriteConfiguration_IsSet_ResetValue(r);
  const bool ResetRequired               = r->ResetRequired;
  const RegisterWriteMode WriteMode      = r->WriteMode;
  const RegisterWriteMode ResetWriteMode = r->ResetWriteMode;

  if (WriteMode == RegisterWriteMode_Call) {
    if (! AcpiMethod)
      return err_stringf(0, "%s: %s", "AcpiMethod", "Missing option");

    if (Value)
      return err_string(0, "Value: Cannot be used with WriteMode == Call");
  }
  else {
    if (! Register)
      return err_stringf(0, "%s: %s", "Register", "Missing option");

    if (! Value)
      return err_stringf(0, "%s: %s", "Value", "Missing option");

    if (AcpiMethod)
      return err_string(0, "AcpiMethod: Cannot be used with WriteMode == Set/And/Or");
  }

  if (ResetRequired) {
    if (ResetWriteMode == RegisterWriteMode_Call) {
      if (! ResetAcpiMethod)
        return err_stringf(0, "%s: %s", "ResetAcpiMethod", "Missing option");

      if (ResetValue)
        return err_string(0, "ResetValue: Cannot be used with ResetWriteMode == Call");
    }
    else {
      if (! Register)
        return err_stringf(0, "%s: %s", "Register", "Missing option");

      if (! ResetValue)
        return err_stringf(0, "%s: %s", "ResetValue", "Missing option");

      if (ResetAcpiMethod)
        return err_string(0, "ResetAcpiMethod: Cannot be used with ResetWriteMode == Set/And/Or");
    }
  }
  else {
    /* This is actually the right behaviour, but too many old config files
     * have ResetValue set even if ResetRequired is false.
     *
    if (ResetValue)
      return err_string(0, "ResetValue: Cannot be used with ResetRequired == false");
    */

    if (ResetAcpiMethod)
      return err_string(0, "ResetAcpiComand: Cannot be used with ResetRequired == false");
  }

  if (WriteMode == RegisterWriteMode_Call && ResetWriteMode == RegisterWriteMode_Call) {
    if (Register)
      return err_string(0, "Register: Cannot be used if both WriteMode == Call and ResetWriteMode == Call");
  }

  return err_success();
}

Error* ModelConfig_Validate(Trace* trace, ModelConfig* c) {
  Error* e;

  e = ModelConfig_ValidateFields(c);
  e_goto(err);

  for_each_array(RegisterWriteConfiguration*, r, c->RegisterWriteConfigurations) {
    Trace_Push(trace, "RegisterWriteConfigurations[%d]", PTR_DIFF(r, c->RegisterWriteConfigurations.data));

    e = RegisterWriteConfiguration_ValidateFields(r);
    e_goto(err);

    e = RegisterWriteConfiguration_Validate(r);
    e_goto(err);

    Trace_Pop(trace);
  }

  for_each_array(FanConfiguration*, f, c->FanConfigurations) {
    Trace_Push(trace, "FanConfigurations[%d]", PTR_DIFF(f, c->FanConfigurations.data));

    e = FanConfiguration_ValidateFields(f);
    e_goto(err);

    // Add a default FanDisplayName
    if (f->FanDisplayName == NULL) {
      char fan_name[32];
      snprintf(fan_name, sizeof(fan_name), "Fan #%d", PTR_DIFF(f, c->FanConfigurations.data));
      f->FanDisplayName = Mem_Strdup(fan_name);
    }

    // If ResetRequired is true, ensure that one (and only one) of "FanSpeedResetValue" and "ResetAcpiMethod" is set
    if (f->ResetRequired) {
      const int reset_group = (FanConfiguration_IsSet_FanSpeedResetValue(f) + FanConfiguration_IsSet_ResetAcpiMethod(f));
      if (reset_group == 0) {
        e = err_stringf(0, "Missing option: %s or %s", "FanSpeedResetValue", "ResetAcpiMethod");
        goto err;
      }
      if (reset_group > 1) {
        e = err_stringf(0, "Cannot set both %s and %s", "FanSpeedResetValue", "ResetAcpiMethod");
        goto err;
      }
    }
    else {
      /* This is actually the right behaviour, but too many old config files
       * have FanSpeedResetValue set even if ResetRequired is false.
       *
      if (FanConfiguration_IsSet_FanSpeedResetValue(f)) {
        e = err_string(0, "FanSpeedResetValue: Cannot be used with ResetRequired == false");
        goto err;
      }
       */

      if (FanConfiguration_IsSet_ResetAcpiMethod(f)) {
        e = err_string(0, "ResetAcpiMethod: Cannot be used with ResetRequired == false");
        goto err;
      }
    }

    // Ensure that one (and only one) of "WriteRegister" and "WriteAcpiMethod" is set
    const int write_group = (FanConfiguration_IsSet_WriteRegister(f) + FanConfiguration_IsSet_WriteAcpiMethod(f));
    if (write_group == 0) {
      e = err_stringf(0, "Missing option: %s or %s", "WriteRegister", "WriteAcpiMethod");
      goto err;
    }
    if (write_group > 1) {
      e = err_stringf(0, "Cannot set both %s and %s", "WriteRegister", "WriteAcpiMethod");
      goto err;
    }

    // Ensure that one (and only one) of "ReadRegister" and "ReadAcpiMethod" is set
    const int read_group = (FanConfiguration_IsSet_ReadRegister(f) + FanConfiguration_IsSet_ReadAcpiMethod(f));
    if (read_group == 0) {
      e = err_stringf(0, "Missing option: %s or %s", "ReadRegister", "ReadAcpiMethod");
      goto err;
    }
    if (read_group > 1) {
      e = err_stringf(0, "Cannot set both %s and %s", "ReadRegister", "ReadAcpiMethod");
      goto err;
    }

    if (f->MinSpeedValue == f->MaxSpeedValue) {
      e = err_stringf(0, "%s and %s cannot be the same", "MinSpeedValue", "MaxSpeedValue");
      goto err;
    }

    if (f->IndependentReadMinMaxValues) {
      if (! FanConfiguration_IsSet_MinSpeedValueRead(f)) {
        e = err_stringf(0, "%s: %s", "MinSpeedValueRead", "Missing option");
        goto err;
      }

      if (! FanConfiguration_IsSet_MaxSpeedValueRead(f)) {
        e = err_stringf(0, "%s: %s", "MaxSpeedValueRead", "Missing option");
        goto err;
      }

      if (f->MinSpeedValueRead == f->MaxSpeedValueRead) {
        e = err_stringf(0, "%s and %s cannot be the same", "MinSpeedValueRead", "MaxSpeedValueRead");
        goto err;
      }
    }

    for_each_array(FanSpeedPercentageOverride* , o, f->FanSpeedPercentageOverrides) {
      Trace_Push(trace, "FanSpeedPercentageOverrides[%d]", PTR_DIFF(o, f->FanSpeedPercentageOverrides.data));

      e = FanSpeedPercentageOverride_ValidateFields(o);
      e_goto(err);

      Trace_Pop(trace);
    }

    if (! f->TemperatureThresholds.size) {
      if (c->LegacyTemperatureThresholdsBehaviour)
        copy_array_of_TemperatureThreshold(
          &f->TemperatureThresholds,
          &Config_DefaultLegacyTemperatureThresholds
        );
      else
        copy_array_of_TemperatureThreshold(
          &f->TemperatureThresholds,
          &Config_DefaultTemperatureThresholds
        );
    }

    e = TemperatureThresholds_Validate(trace, &f->TemperatureThresholds, c->CriticalTemperature);
    e_goto(err);

    Trace_Pop(trace);
  }

  return err_success();

err:
  return err_string(e, trace->buf);
}

Error* ModelConfig_FromFile(ModelConfig* config, const char* file) {
  Error* e;
  char file_content[NBFC_MAX_FILE_SIZE];
  char nxjson_memory[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;

  // Use memory from the stack to allocate data structures from nxjson
  StackMemory_Init(nxjson_memory, sizeof(nxjson_memory));

  e = nx_json_parse_file(&js, file_content, sizeof(file_content), file);
  if (e)
    goto err;

  e = ModelConfig_FromJson(config, js);
  if (e)
    goto err;

err:
  nx_json_free(js);
  StackMemory_Destroy();
  return e;
}

Error* ModelConfig_FindAndLoad(ModelConfig* config, char* resolved, const char* file) {
  if (file[0] == '/') {
    snprintf(resolved, PATH_MAX, "%s", file);
    return ModelConfig_FromFile(config, file);
  }

  snprintf(resolved, PATH_MAX, "%s/%s.json", NBFC_MODEL_CONFIGS_DIR_MUTABLE, file);
  if (access(resolved, F_OK) == 0)
    return ModelConfig_FromFile(config, resolved);

  snprintf(resolved, PATH_MAX, "%s/%s.json", NBFC_MODEL_CONFIGS_DIR, file);
  return ModelConfig_FromFile(config, resolved);
}
