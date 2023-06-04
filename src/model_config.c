#include "model_config.h"

#include "nbfc.h"
#include "memory.h"
#include "nxjson_utils.h"

#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#define str_Unset   NULL
#define int_Unset   INT_MIN
#define short_Unset SHRT_MIN
#define float_Unset NAN

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
  return err_string(0, "Not a int");
}

static inline Error* short_FromJson(short* out, const nx_json* node) {
  int val = 0;
  Error* e = int_FromJson(&val, node);
  e_check();
  if (val < SHRT_MIN || val > SHRT_MAX)
    return err_string(0, "Value not in range for short type");
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

static inline Error* Boolean_FromJson(Boolean* out, const nx_json* json) {
  return bool_FromJson((bool*) out, json);
}

static inline Error* str_FromJson(const char** out, const nx_json* json) {
  Error* e = nx_json_get_str(out, json);
  e_check();
  *out = Mem_Strdup(*out);
  return err_success();
}

static Error* RegisterWriteMode_FromJson(RegisterWriteMode* out, const nx_json* json) {
  const char* s = NULL;
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  else if (!strcmp(s, "Set")) *out = RegisterWriteMode_Set;
  else if (!strcmp(s, "And")) *out = RegisterWriteMode_And;
  else if (!strcmp(s, "Or"))  *out = RegisterWriteMode_Or;
  else return err_string(0, "Invalid value for RegisterWriteMode");
  return e;
}

static Error* RegisterWriteOccasion_FromJson(RegisterWriteOccasion* out, const nx_json* json) {
  const char* s = NULL;
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  else if (!strcmp(s, "OnWriteFanSpeed"))  *out = RegisterWriteOccasion_OnWriteFanSpeed;
  else if (!strcmp(s, "OnInitialization")) *out = RegisterWriteOccasion_OnInitialization;
  else return err_string(0, "Invalid value for RegisterWriteOccasion");
  return e;
}

static Error* OverrideTargetOperation_FromJson(OverrideTargetOperation* out, const nx_json* json) {
  const char* s = NULL;
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  else if (!strcmp(s, "Read"))       *out = OverrideTargetOperation_Read;
  else if (!strcmp(s, "Write"))      *out = OverrideTargetOperation_Write;
  else if (!strcmp(s, "ReadWrite"))  *out = OverrideTargetOperation_ReadWrite;
  else return err_string(0, "Invalid value for OverrideTargetOperation");
  return e;
}

static Error* EmbeddedControllerType_FromJson(EmbeddedControllerType* out, const nx_json* json) {
  const char* s = NULL;
  Error* e = nx_json_get_str(&s, json);
  if (e) return e;
  EmbeddedControllerType t = EmbeddedControllerType_FromString(s);
  if (t == EmbeddedControllerType_Unset)
    return err_string(0, "Invalid value for EmbeddedControllerType");
  *out = t;
  return e;
}

EmbeddedControllerType EmbeddedControllerType_FromString(const char* s) {
  if (!strcmp(s, "ec_sys_linux")) return EmbeddedControllerType_ECSysLinux;
  if (!strcmp(s, "ec_acpi"))      return EmbeddedControllerType_ECSysLinuxACPI;
  if (!strcmp(s, "ec_linux"))     return EmbeddedControllerType_ECLinux;
  if (!strcmp(s, "dummy"))        return EmbeddedControllerType_ECDummy;
  return EmbeddedControllerType_Unset;
}

const char* EmbeddedControllerType_ToString(EmbeddedControllerType t) {
  switch (t) {
  case EmbeddedControllerType_ECSysLinux:     return "ec_sys_linux";
  case EmbeddedControllerType_ECSysLinuxACPI: return "ec_acpi";
  case EmbeddedControllerType_ECLinux:        return "ec_linux";
  case EmbeddedControllerType_ECDummy:        return "dummy";
  default: assert(!"Invalid value for EmbeddedControllerType");
  }
  return NULL;
}

typedef Error* (FromJson_Callback)(void*, const nx_json*);

static Error* array_of_FromJson(FromJson_Callback callback, void** v_data, size_t* v_size, size_t size, const nx_json* json) {
  Error* e = nx_json_get_array(json);
  e_check();

  *v_size = 0;
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

define_array_of_T_FromJson(float)
define_array_of_T_FromJson(TemperatureThreshold)
define_array_of_T_FromJson(FanConfiguration)
define_array_of_T_FromJson(FanSpeedPercentageOverride)
define_array_of_T_FromJson(RegisterWriteConfiguration)
define_array_of_T_FromJson(FanInfo)

// ============================================================================
// Default temperature thresholds
// ============================================================================

static TemperatureThreshold ___Config_DefaultTemperatureThresholds[] = {
  {60,  0,   0},
  {63, 48,  10},
  {66, 55,  20},
  {68, 59,  50},
  {71, 63,  70},
  {75, 67, 100},
};

static array_of(TemperatureThreshold) Config_DefaultTemperatureThresholds = {
  ___Config_DefaultTemperatureThresholds,
  ARRAY_SIZE(___Config_DefaultTemperatureThresholds)
};

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
    Mem_Free(f->TemperatureThresholds.data);
    Mem_Free(f->FanSpeedPercentageOverrides.data);
  }

  Mem_Free(c->FanConfigurations.data);

  for_each_array(RegisterWriteConfiguration*, r, c->RegisterWriteConfigurations) {
    Mem_Free((char*) r->Description);
  }

  Mem_Free(c->RegisterWriteConfigurations.data);

  memset(c, 0, sizeof(*c));
}

struct Trace {
  char text[128];
  struct Trace* next;
};
typedef struct Trace Trace;

static void Trace_PrintBuf(const Trace* trace, char* buf, size_t size) {
  int buf_written = 0;
  while (trace) {
    if (trace->text[0]) {
      const int written = snprintf(buf + buf_written, size - buf_written, "%s: ", trace->text);
      if (written >= size - buf_written)
        return;
      buf_written += written;
    }

    trace = trace->next;
  }

  if (buf_written)
    buf[buf_written - 2] = '\0'; // strip ": "
}

// ============================================================================
// Validation code
// ============================================================================
//
// Calls *_ValidateFields on each structure and does some validations
// that cannot be auto-generated.

Error* ModelConfig_Validate(ModelConfig* c) {
  Error* e = NULL;
  Trace trace = {0};
  static char buf[128];

  e = ModelConfig_ValidateFields(c);
  e_goto(err);

  for_each_array(RegisterWriteConfiguration*, r, c->RegisterWriteConfigurations) {
    snprintf(trace.text, sizeof(trace.text), "RegisterWriteConfigurations[%td]", r - c->RegisterWriteConfigurations.data);

    // Don't make the validation fail if `ResetRequired` is false and `ResetValue` was not set
    if (r->ResetRequired == Boolean_False || r->ResetRequired == Boolean_Unset)
      r->ResetValue = 0;

    e = RegisterWriteConfiguration_ValidateFields(r);
    e_goto(err);

    trace.text[0] = 0;
  }

  for_each_array(FanConfiguration*, f, c->FanConfigurations) {
    snprintf(trace.text, sizeof(trace.text), "FanConfigurations[%td]", f - c->FanConfigurations.data);

    // Don't make the validation fail if `ResetRequired` is false and `FanSpeedResetValue` was not set
    if (f->ResetRequired == Boolean_False || f->ResetRequired == Boolean_Unset)
      f->FanSpeedResetValue = 0;

    e = FanConfiguration_ValidateFields(f);
    e_goto(err);

    for_each_array(FanSpeedPercentageOverride* , o, f->FanSpeedPercentageOverrides) {
      Trace trace1 = {0};
      snprintf(trace1.text, sizeof(trace1.text), "FanSpeedPercentageOverrides[%td]", o - f->FanSpeedPercentageOverrides.data);
      trace.next = &trace1;

      e = FanSpeedPercentageOverride_ValidateFields(o);
      e_goto(err);

      trace.next = NULL;
    }

    // TODO #1: How to handle empty TemperatureThresholds? [see fan.c]
    if (! f->TemperatureThresholds.size)
      f->TemperatureThresholds = Config_DefaultTemperatureThresholds;

    bool has_0_FanSpeed   = false;
    bool has_100_FanSpeed = false;

    for_each_array(TemperatureThreshold*, t, f->TemperatureThresholds) {
      Trace trace1 = {0};
      snprintf(trace1.text, sizeof(trace1.text), "TemperatureThresholds[%td]", t - f->TemperatureThresholds.data);
      trace.next = &trace1;

      e = TemperatureThreshold_ValidateFields(t);
      e_goto(err);

      has_0_FanSpeed   |= (t->FanSpeed == 0);
      has_100_FanSpeed |= (t->FanSpeed == 100);

      if (t->UpThreshold < t->DownThreshold) {
        e = err_string(0, "UpThreshold cannot be less than DownThreshold");
        goto err;
      }

      if (t->UpThreshold > c->CriticalTemperature) {
        Trace_PrintBuf(&trace, buf, sizeof(buf));
        e = err_string(0, "UpThreshold cannot be greater than CriticalTemperature");
        e = err_string(e, buf);
        e_warn();
        e = NULL;
      }

      for_each_array(TemperatureThreshold*, t1, f->TemperatureThresholds) {
        if (t != t1 && t->UpThreshold == t1->UpThreshold) {
          e = err_string(0, "Duplicate UpThreshold");
          goto err;
        }
      }

      trace.next = NULL;
    }

    if (! has_0_FanSpeed) {
      Trace_PrintBuf(&trace, buf, sizeof(buf));
      e = err_string(0, "No threshold with FanSpeed == 0 found");
      e = err_string(e, buf);
      e_warn();
      e = NULL;
    }

    if (! has_100_FanSpeed) {
      Trace_PrintBuf(&trace, buf, sizeof(buf));
      e = err_string(0, "No threshold with FanSpeed == 100 found");
      e = err_string(e, buf);
      e_warn();
      e = NULL;
    }
  }

err:
  if (! e)
    return e;

  Trace_PrintBuf(&trace, buf, sizeof(buf));
  return err_string(e, buf);
}

Error* ModelConfig_FromFile(ModelConfig* config, const char* file) {
  char buf[NBFC_MAX_FILE_SIZE];
  const nx_json* js = NULL;
  Error* e = nx_json_parse_file(&js, buf, sizeof(buf), file);
  e_check();
  e = ModelConfig_FromJson(config, js);
  nx_json_free(js);
  return e;
}
