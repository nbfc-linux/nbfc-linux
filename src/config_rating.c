#include "config_rating.h"
#include "nxjson_utils.h"
#include "memory.h"

#include <stdio.h>  // printf, snprintf
#include <string.h> // memset, strcmp, strstr, strlen

/*
 * Parses the JSON string and populates the `ConfigRatingRules` structure.
 *
 * Returns an error if:
 *  - The JSON itself is invalid.
 *  - The JSON structure is invalid (invalid fields, invalid types).
 */
Error ConfigRatingRules_FromJson(ConfigRatingRules* rules, const char* rules_json) {
  Error e = err_success();
  char* rules_json_temp = NULL;
  const nx_json* root = NULL;

  memset(rules, 0, sizeof(*rules));
  rules_json_temp = Mem_Strdup(rules_json);
  root = nx_json_parse_utf8(rules_json_temp);
  
  if (! root) {
    e = err_nxjson(NULL);
    goto end;
  }

  if (root->type != NX_JSON_OBJECT) {
    e = err_string("Not a JSON object");
    goto end;
  }

#define READ_REGISTER_ARRAY(PARENT_KEY, KEY, FIELD)                            \
  if (! strcmp(child->key, KEY)) {                                             \
    if (child->type != NX_JSON_ARRAY) {                                        \
      e = err_stringf("%s: %s: Not a JSON array", PARENT_KEY, KEY);            \
      goto end;                                                                \
    }                                                                          \
                                                                               \
    rules->FIELD.size = 0;                                                     \
    rules->FIELD.data = Mem_Realloc(                                           \
      rules->FIELD.data,                                                       \
      child->val.children.length * sizeof(AcpiRegisterName));                  \
                                                                               \
    nx_json_for_each(register_name, child) {                                   \
      if (register_name->type != NX_JSON_STRING) {                             \
        e = err_stringf("%s: %s[%d]: Not a string",                            \
              PARENT_KEY, KEY, rules->FIELD.size);                             \
        goto end;                                                              \
      }                                                                        \
                                                                               \
      if (strlen(register_name->val.text) > sizeof(AcpiRegisterName) - 1) {    \
        e = err_stringf("%s: %s[%d]: %s: Invalid length",                      \
              PARENT_KEY, KEY, rules->FIELD.size, register_name->val.text);    \
        goto end;                                                              \
      }                                                                        \
                                                                               \
      snprintf(                                                                \
        rules->FIELD.data[rules->FIELD.size],                                  \
        sizeof(AcpiRegisterName),                                              \
        "%s",                                                                  \
        register_name->val.text);                                              \
                                                                               \
      rules->FIELD.size++;                                                     \
    }                                                                          \
                                                                               \
    continue;                                                                  \
  }

  nx_json_for_each(category, root) {
    if (! strcmp(category->key, "FanConfiguration")) {
      if (category->type != NX_JSON_OBJECT) {
        e = err_stringf("%s: Not a JSON object", "FanConfiguration");
        goto end;
      }

      nx_json_for_each(child, category) {
        READ_REGISTER_ARRAY("FanConfiguration", "RegisterFullMatch", FanRegistersFullMatch)
        READ_REGISTER_ARRAY("FanConfiguration", "RegisterPartialMatch", FanRegistersPartialMatch)
      
        e = err_stringf("%s: Unknown key: %s", "FanConfiguration", child->key);
        goto end;
      }

      continue;
    }

    if (! strcmp(category->key, "RegisterWriteConfiguration")) {
      if (category->type != NX_JSON_OBJECT) {
        e = err_stringf("%s: Not a JSON object", "RegisterWriteConfiguration");
        goto end;
      }

      nx_json_for_each(child, category) {
        READ_REGISTER_ARRAY("RegisterWriteConfiguration", "RegisterFullMatch", RegisterWriteFullMatch)
        READ_REGISTER_ARRAY("RegisterWriteConfiguration", "RegisterPartialMatch", RegisterWritePartialMatch)
      
        e = err_stringf("%s: Unknown key: %s", "RegisterWriteConfiguration", child->key);
        goto end;
      }

      continue;
    }

    e = err_stringf("Unknown key: %s", category->key);
    goto end;
  }

end:
  nx_json_free(root);
  Mem_Free(rules_json_temp);

  if (e)
    ConfigRatingRules_Free(rules);

  return e;
}

void ConfigRatingRules_Free(ConfigRatingRules* rules) {
  Mem_Free(rules->FanRegistersFullMatch.data);
  Mem_Free(rules->FanRegistersPartialMatch.data);
  Mem_Free(rules->RegisterWriteFullMatch.data);
  Mem_Free(rules->RegisterWritePartialMatch.data);
  memset(rules, 0, sizeof(*rules));
}

Error ConfigRating_Init(ConfigRating* config_rating, const char* dsdt_file, const char* rules_json) {
  Error e;
  memset(config_rating, 0, sizeof(*config_rating));

  e = ConfigRatingRules_FromJson(&config_rating->rules, rules_json);
  if (e) {
    e = err_chain_string(e, "Rules");
    goto end;
  }

  e = Acpi_Analysis_Get_Registers(dsdt_file, &config_rating->registers);
  if (e)
    goto end;

  e = Acpi_Analysis_Get_Methods(dsdt_file, &config_rating->methods);
  if (e)
    goto end;

  e = Acpi_Analysis_Get_EC_OperationRegions(dsdt_file, &config_rating->ec_operation_regions);
  if (e)
    goto end;

end:
  if (e)
    ConfigRating_Free(config_rating);

  return e;
}

void ConfigRating_Free(ConfigRating* config_rating) {
  for_each_array(AcpiMethod*, method, config_rating->methods) {
    AcpiMethod_Free(method);
  }
  Mem_Free(config_rating->methods.data);

  for_each_array(AcpiRegister*, register_, config_rating->registers) {
    AcpiRegister_Free(register_);
  }
  Mem_Free(config_rating->registers.data);

  Mem_Free(config_rating->ec_operation_regions.data);

  ConfigRatingRules_Free(&config_rating->rules);

  memset(config_rating, 0, sizeof(*config_rating));
}

static bool ConfigRating_IsKnownFanRegister(ConfigRating* config_rating, const char* s) {
  for_each_array(AcpiRegisterName*, name, config_rating->rules.FanRegistersFullMatch)
    if (! strcmp(s, *name))
      return true;

  return false;
}

static bool ConfigRating_IsSomeFanRegister(ConfigRating* config_rating, const char* s) {
  for_each_array(AcpiRegisterName*, name, config_rating->rules.FanRegistersPartialMatch)
    if (strstr(s, *name))
      return true;

  return false;
}

static bool ConfigRating_IsKnownRegisterWriteConfigRegister(ConfigRating* config_rating, const char* s) {
  for_each_array(AcpiRegisterName*, name, config_rating->rules.RegisterWriteFullMatch)
    if (! strcmp(s, *name))
      return true;

  return false;
}

static bool ConfigRating_IsSomeRegisterWriteConfigRegister(ConfigRating* config_rating, const char* s) {
  for_each_array(AcpiRegisterName*, name, config_rating->rules.RegisterWritePartialMatch)
    if (strstr(s, *name))
      return true;

  return false;
}

static bool ConfigRating_IsMinimalFanRegister(const char* s) {
  return (s[0] == 'F');
}

static bool ConfigRating_IsBadRegister(const char* s) {
  return (s[0] == 'B');
}

static AcpiRegister* ConfigRating_FindEcRegister(
  ConfigRating* config_rating,
  int offset
) {
  for_each_array(AcpiRegister*, acpi_register, config_rating->registers) {
    if ((acpi_register->bit_offset / 8) != offset)
      continue;

    for_each_array(AcpiOperationRegion*, region, config_rating->ec_operation_regions) {
      if (! strcmp(acpi_register->region, *region)) {
        return acpi_register;
      }
    }
  }

  return NULL;
}

static AcpiMethod* ConfigRating_FindMethod(
  ConfigRating* config_rating,
  const char* method_call
) {
  for_each_array(AcpiMethod*, method, config_rating->methods) {
    if (Acpi_Analysis_Path_Equals(method_call, method->name)) {
      return method;
    }
  }

  return NULL;
}

static ConfigRating_RegisterRating ConfigRating_RateRegister(
  ConfigRating* config_rating,
  enum RegisterType register_type,
  int offset
) {
  ConfigRating_RegisterRating rated = {0};
  rated.register_type = register_type;
  rated.register_offset = offset;
  rated.acpi_register = ConfigRating_FindEcRegister(config_rating, offset);

  if (! rated.acpi_register) {
    rated.register_score = RegisterScore_NotFound;
    goto ret;
  }

  if (rated.acpi_register->bit_offset % 8 == 0)
    rated.register_alignment = RegisterAlignment_OK;
  else
    rated.register_alignment = RegisterAlignment_Misaligned;

  const char* const name = Acpi_Analysis_Get_Register_Basename(rated.acpi_register->name);

  if (register_type == RegisterType_FanRegister) {
    if (ConfigRating_IsKnownFanRegister(config_rating, name)) {
      rated.register_score = RegisterScore_FullMatch;
      goto ret;
    }

    if (ConfigRating_IsBadRegister(name)) {
      rated.register_score = RegisterScore_BadRegister;
      goto ret;
    }

    if (ConfigRating_IsSomeFanRegister(config_rating, name)) {
      rated.register_score = RegisterScore_PartialMatch;
      goto ret;
    }

    if (ConfigRating_IsMinimalFanRegister(name)) {
      rated.register_score = RegisterScore_MinimalMatch;
      goto ret;
    }

    rated.register_score = RegisterScore_NoMatch;
  }
  else {
    if (ConfigRating_IsKnownRegisterWriteConfigRegister(config_rating, name)) {
      rated.register_score = RegisterScore_FullMatch;
      goto ret;
    }

    if (ConfigRating_IsBadRegister(name)) {
      rated.register_score = RegisterScore_BadRegister;
      goto ret;
    }

    if (ConfigRating_IsSomeRegisterWriteConfigRegister(config_rating, name)) {
      rated.register_score = RegisterScore_PartialMatch;
      goto ret;
    }

    if (ConfigRating_IsMinimalFanRegister(name)) {
      rated.register_score = RegisterScore_MinimalMatch;
      goto ret;
    }

    rated.register_score = RegisterScore_NoMatch;
  }

ret:
  return rated;
}

static ConfigRating_MethodRating ConfigRating_RateMethod(
  ConfigRating* config_rating,
  const char* method_call
) {
  ConfigRating_MethodRating rated = {0};
  rated.method_call = Mem_Strdup(method_call);
  rated.acpi_method = ConfigRating_FindMethod(config_rating, method_call);

  if (rated.acpi_method)
    rated.method_score = MethodScore_Found;
  else
    rated.method_score = MethodScore_NotFound;

  return rated;
}

static void ConfigRating_RegisterRatingPrint(ConfigRating_RegisterRating* rating) {
  printf("\tEC Register %d (0x%X):\n", rating->register_offset, rating->register_offset);

  switch (rating->register_type) {
  case RegisterType_FanRegister:
    printf("\t\tType:   Fan register\n");
    break;
  case RegisterType_RegisterWriteConfigurationRegister:
    printf("\t\tType:   RegisterWriteConfiguration register\n");
    break;
  }

  if (rating->register_score != RegisterScore_NotFound) {
    printf("\t\tName:   %s\n", rating->acpi_register->name);
    printf("\t\tLength: %d\n", rating->acpi_register->bit_length);
  }

  switch (rating->register_score) {
    case RegisterScore_FullMatch:
      printf("\t\tRating: FULL MATCH\n");
      break;

    case RegisterScore_PartialMatch:
      printf("\t\tRating: PARTIAL MATCH\n");
      break;

    case RegisterScore_MinimalMatch:
      printf("\t\tRating: MINIMAL MATCH\n");
      break;

    case RegisterScore_BadRegister:
      printf("\t\tRating: BAD REGISTER\n");
      break;

    case RegisterScore_NoMatch:
      printf("\t\tRating: NO MATCH\n");
      break;

    case RegisterScore_NotFound:
      printf("\t\tRating: NOT FOUND\n");
      break;
  }
}

static void ConfigRating_MethodRatingPrint(ConfigRating_MethodRating* rating) {
  printf("\tACPI Method \"%s\":\n", rating->method_call);

  switch (rating->method_score) {
    case MethodScore_Found:
      printf("\t\tRating: FULL MATCH\n");
      break;

    case MethodScore_NotFound:
      printf("\t\tRating: NOT FOUND\n");
      break;
  }
}

void ConfigRating_RatingPrint(ConfigRating_Rating* rating) {
  printf("\tConfig score: %.2f / 10.00\n", rating->score);

  for_each_array(ConfigRating_RegisterRating*, reg_rating, rating->register_ratings)
    ConfigRating_RegisterRatingPrint(reg_rating);

  for_each_array(ConfigRating_MethodRating*, met_rating, rating->method_ratings)
    ConfigRating_MethodRatingPrint(met_rating);
}

void ConfigRating_RateModelConfig(
  ConfigRating* config_rating,
  ModelConfig* model_config,
  ConfigRating_Rating *rating)
{
  // ==========================================================================
  // Calculate output size of `rating` arrays
  // ==========================================================================

  array_size_t registers_size = 0;
  array_size_t methods_size = 0;

  for_each_array(FanConfiguration*, fan_config, model_config->FanConfigurations) {
    registers_size += FanConfiguration_IsSet_ReadRegister(fan_config);
    registers_size += FanConfiguration_IsSet_WriteRegister(fan_config);
    methods_size += FanConfiguration_IsSet_ReadAcpiMethod(fan_config);
    methods_size += FanConfiguration_IsSet_WriteAcpiMethod(fan_config);
    methods_size += FanConfiguration_IsSet_ResetAcpiMethod(fan_config);
  }

  for_each_array(RegisterWriteConfiguration*, rw_config, model_config->RegisterWriteConfigurations) {
    registers_size += RegisterWriteConfiguration_IsSet_Register(rw_config);
    methods_size += RegisterWriteConfiguration_IsSet_AcpiMethod(rw_config);
    methods_size += RegisterWriteConfiguration_IsSet_ResetAcpiMethod(rw_config);
  }

  // ==========================================================================
  // Allocate memory for arrays
  // ==========================================================================

  rating->register_ratings.size = 0;
  rating->register_ratings.data = Mem_Calloc(registers_size, sizeof(ConfigRating_RegisterRating));

  rating->method_ratings.size = 0;
  rating->method_ratings.data = Mem_Calloc(methods_size, sizeof(ConfigRating_MethodRating));

  // ==========================================================================
  // Do the actual rating
  // ==========================================================================

  for_each_array(FanConfiguration*, fan_config, model_config->FanConfigurations) {
    // Registers
    if (FanConfiguration_IsSet_ReadRegister(fan_config))
      rating->register_ratings.data[rating->register_ratings.size++] = \
        ConfigRating_RateRegister(config_rating, RegisterType_FanRegister, fan_config->ReadRegister);

    if (FanConfiguration_IsSet_WriteRegister(fan_config))
      rating->register_ratings.data[rating->register_ratings.size++] = \
        ConfigRating_RateRegister(config_rating, RegisterType_FanRegister, fan_config->WriteRegister);

    // Methods
    if (FanConfiguration_IsSet_ReadAcpiMethod(fan_config))
      rating->method_ratings.data[rating->method_ratings.size++] = \
        ConfigRating_RateMethod(config_rating, fan_config->ReadAcpiMethod);

    if (FanConfiguration_IsSet_WriteAcpiMethod(fan_config))
      rating->method_ratings.data[rating->method_ratings.size++] = \
        ConfigRating_RateMethod(config_rating, fan_config->WriteAcpiMethod);

    if (FanConfiguration_IsSet_ResetAcpiMethod(fan_config))
      rating->method_ratings.data[rating->method_ratings.size++] = \
        ConfigRating_RateMethod(config_rating, fan_config->ResetAcpiMethod);
  }

  for_each_array(RegisterWriteConfiguration*, rw_config, model_config->RegisterWriteConfigurations) {
    // Registers
    if (RegisterWriteConfiguration_IsSet_Register(rw_config))
      rating->register_ratings.data[rating->register_ratings.size++] = \
        ConfigRating_RateRegister(config_rating, RegisterType_RegisterWriteConfigurationRegister, rw_config->Register);

    // Methods
    if (RegisterWriteConfiguration_IsSet_AcpiMethod(rw_config))
      rating->method_ratings.data[rating->method_ratings.size++] = \
        ConfigRating_RateMethod(config_rating, rw_config->AcpiMethod);

    if (RegisterWriteConfiguration_IsSet_ResetAcpiMethod(rw_config))
      rating->method_ratings.data[rating->method_ratings.size++] = \
        ConfigRating_RateMethod(config_rating, rw_config->ResetAcpiMethod);
  }

  // ==========================================================================
  // Calculate the score
  // ==========================================================================
  
  int points = 0;
  int priority = (registers_size + methods_size);

  for_each_array(ConfigRating_RegisterRating*, reg_rating, rating->register_ratings) {
    int register_points = 0;

    if (reg_rating->register_type == RegisterType_FanRegister) {
      switch (reg_rating->register_score) {
        case RegisterScore_FullMatch:      register_points = 10; break;
        case RegisterScore_PartialMatch:   register_points = 9;  break;
        case RegisterScore_MinimalMatch:   register_points = 7;  break;
        case RegisterScore_NoMatch:        register_points = 0;  break;
        case RegisterScore_NotFound:       register_points = 0;  break;
        case RegisterScore_BadRegister:    points = 0;           goto end;
      }

      if (reg_rating->register_score != RegisterScore_NotFound) {
        if (reg_rating->register_alignment == RegisterAlignment_Misaligned) {
          register_points = 0;
        }

        const int bit_length = reg_rating->acpi_register->bit_length;
        if (bit_length != 8 && bit_length != 16) {
          register_points = 0;
        }
      }
    }
    else {
      switch (reg_rating->register_score) {
        case RegisterScore_FullMatch:      register_points = 10; break;
        case RegisterScore_PartialMatch:   register_points = 9;  break;
        case RegisterScore_MinimalMatch:   register_points = 7;  break;
        case RegisterScore_NoMatch:        register_points = 2;  break;
        case RegisterScore_NotFound:       register_points = 0;  break;
        case RegisterScore_BadRegister:    points = 0;           goto end;
      }
    }

    points += register_points;
  }

  for_each_array(ConfigRating_MethodRating*, met_rating, rating->method_ratings) {
    int method_points = 0;

    switch (met_rating->method_score) {
      case MethodScore_Found:
        method_points = 10;
        priority += 10;
        break;

      case MethodScore_NotFound:
        points = 0;
        goto end;
    }

    points += method_points;
  }

end:
  rating->score = (float) points / (registers_size + methods_size);
  rating->priority = priority;
}

void ConfigRating_RegisterRatingFree(ConfigRating_RegisterRating* rating) {
  (void) rating;
}

void ConfigRating_MethodRatingFree(ConfigRating_MethodRating* rating) {
  Mem_Free(rating->method_call);
}

void ConfigRating_RatingFree(ConfigRating_Rating* rating) {
  for_each_array(ConfigRating_RegisterRating*, reg_rating, rating->register_ratings)
    ConfigRating_RegisterRatingFree(reg_rating);

  for_each_array(ConfigRating_MethodRating*, met_rating, rating->method_ratings)
    ConfigRating_MethodRatingFree(met_rating);

  memset(rating, 0, sizeof(*rating));
}
