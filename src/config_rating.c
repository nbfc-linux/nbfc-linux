#include "config_rating.h"
#include "nxjson_utils.h"
#include "memory.h"

#include <stdio.h>  // printf
#include <string.h> // memset, strcmp, strstr

Error ConfigRating_Init(ConfigRating* config_rating, const char* dsdt_file, const char* rules_json) {
  Error e;
  memset(config_rating, 0, sizeof(*config_rating));

  e = ConfigRatingRules_FromJson(&config_rating->rules, rules_json);
  if (e) {
    e = err_chain_string(e, "Rules");
    goto end;
  }

  e = Acpi_Analysis_Get_Info(dsdt_file, &config_rating->acpi_info);
  if (e)
    goto end;

end:
  if (e)
    ConfigRating_Free(config_rating);

  return e;
}

void ConfigRating_Free(ConfigRating* config_rating) {
  AcpiInfo_Free(&config_rating->acpi_info);
  ConfigRatingRules_Free(&config_rating->rules);
  memset(config_rating, 0, sizeof(*config_rating));
}

enum RegisterMatchType {
  RegisterMatchType_NoMatch,
  RegisterMatchType_NoModeMatch,
  RegisterMatchType_FullMatch,
};

static enum RegisterMatchType ConfigRating_IsKnownFanRegister(
  ConfigRating* config_rating,
  enum RegisterType type,
  const char* s)
{
  for_each_array(RegisterRule*, rule, config_rating->rules.FanRegistersFullMatch) {
    if (! strcmp(s, rule->Name)) {
      if (type == RegisterType_FanReadRegister && rule->Mode & RegisterRuleFanMode_Read)
        return RegisterMatchType_FullMatch;

      if (type == RegisterType_FanWriteRegister && rule->Mode & RegisterRuleFanMode_Write)
        return RegisterMatchType_FullMatch;

      return RegisterMatchType_NoModeMatch;
    }
  }

  return RegisterMatchType_NoMatch;
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
  for_each_array(AcpiRegister*, acpi_register, config_rating->acpi_info.registers) {
    if ((acpi_register->bit_offset / 8) != offset)
      continue;

    for_each_array(AcpiOperationRegionName*, region, config_rating->acpi_info.ec_region_names) {
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
  for_each_array(AcpiMethod*, method, config_rating->acpi_info.methods) {
    if (Acpi_Analysis_Path_Equals(method_call, method->name)) {
      return method;
    }
  }

  return NULL;
}

static bool ConfigRating_RegisterIsByteAligned(ConfigRating_RegisterRating* rating) {
  if (! rating->info)
    return false;

  return (rating->info->bit_offset % 8 == 0);
}

static ConfigRating_RegisterRating ConfigRating_RateRegister(
  ConfigRating* config_rating,
  enum RegisterType type,
  int offset
) {
  ConfigRating_RegisterRating rated = {0};
  rated.type = type;
  rated.offset = offset;
  rated.info= ConfigRating_FindEcRegister(config_rating, offset);

  if (! rated.info) {
    rated.score = RegisterScore_NotFound;
    goto ret;
  }

  const char* const name = Acpi_Analysis_Get_Register_Basename(rated.info->name);

  if (type == RegisterType_FanReadRegister || type == RegisterType_FanWriteRegister) {
    enum RegisterMatchType match = ConfigRating_IsKnownFanRegister(config_rating, type, name);
    switch (match) {
      case RegisterMatchType_NoMatch:
        break;
      case RegisterMatchType_FullMatch:
        rated.score = RegisterScore_FullMatch;
        goto ret;
      case RegisterMatchType_NoModeMatch:
        rated.score = RegisterScore_NoMatch;
        goto ret;
    }

    if (ConfigRating_IsBadRegister(name)) {
      rated.score = RegisterScore_BadRegister;
      goto ret;
    }

    if (ConfigRating_IsSomeFanRegister(config_rating, name)) {
      rated.score = RegisterScore_PartialMatch;
      goto ret;
    }

    if (ConfigRating_IsMinimalFanRegister(name)) {
      rated.score = RegisterScore_MinimalMatch;
      goto ret;
    }

    rated.score = RegisterScore_NoMatch;
  }
  else {
    if (ConfigRating_IsKnownRegisterWriteConfigRegister(config_rating, name)) {
      rated.score = RegisterScore_FullMatch;
      goto ret;
    }

    if (ConfigRating_IsBadRegister(name)) {
      rated.score = RegisterScore_BadRegister;
      goto ret;
    }

    if (ConfigRating_IsSomeRegisterWriteConfigRegister(config_rating, name)) {
      rated.score = RegisterScore_PartialMatch;
      goto ret;
    }

    if (ConfigRating_IsMinimalFanRegister(name)) {
      rated.score = RegisterScore_MinimalMatch;
      goto ret;
    }

    rated.score = RegisterScore_NoMatch;
  }

ret:
  return rated;
}

static ConfigRating_MethodRating ConfigRating_RateMethod(
  ConfigRating* config_rating,
  const char* method_call
) {
  ConfigRating_MethodRating rated = {0};
  rated.call = Mem_Strdup(method_call);
  rated.info = ConfigRating_FindMethod(config_rating, method_call);

  if (rated.info)
    rated.score = MethodScore_Found;
  else
    rated.score = MethodScore_NotFound;

  return rated;
}

static void ConfigRating_RegisterRatingPrint(ConfigRating_RegisterRating* rating) {
  printf("\tEC Register %d (0x%X):\n", rating->offset, rating->offset);

  switch (rating->type) {
  case RegisterType_FanReadRegister:
    printf("\t\tType:   Fan read register\n");
    break;
  case RegisterType_FanWriteRegister:
    printf("\t\tType:   Fan write register\n");
    break;
  case RegisterType_RegisterWriteConfigurationRegister:
    printf("\t\tType:   RegisterWriteConfiguration register\n");
    break;
  }

  if (rating->score != RegisterScore_NotFound) {
    printf("\t\tName:   %s\n", rating->info->name);
    printf("\t\tLength: %d\n", rating->info->bit_length);
  }

  switch (rating->score) {
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
  printf("\tACPI Method \"%s\":\n", rating->call);

  switch (rating->score) {
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
        ConfigRating_RateRegister(config_rating, RegisterType_FanReadRegister, fan_config->ReadRegister);

    if (FanConfiguration_IsSet_WriteRegister(fan_config))
      rating->register_ratings.data[rating->register_ratings.size++] = \
        ConfigRating_RateRegister(config_rating, RegisterType_FanWriteRegister, fan_config->WriteRegister);

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

    if (reg_rating->type == RegisterType_FanReadRegister || reg_rating->type == RegisterType_FanWriteRegister) {
      switch (reg_rating->score) {
        case RegisterScore_FullMatch:      register_points = 10; break;
        case RegisterScore_PartialMatch:   register_points = 9;  break;
        case RegisterScore_MinimalMatch:   register_points = 7;  break;
        case RegisterScore_NoMatch:        register_points = 0;  break;
        case RegisterScore_NotFound:       register_points = 0;  break;
        case RegisterScore_BadRegister:    points = 0;           goto end;
      }

      if (reg_rating->score != RegisterScore_NotFound) {
        if (! ConfigRating_RegisterIsByteAligned(reg_rating)) {
          register_points = 0;
        }

        const int bit_length = reg_rating->info->bit_length;
        if (bit_length != 8 && bit_length != 16) {
          register_points = 0;
        }
      }
    }
    else {
      switch (reg_rating->score) {
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

    switch (met_rating->score) {
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
  Mem_Free(rating->call);
}

void ConfigRating_RatingFree(ConfigRating_Rating* rating) {
  for_each_array(ConfigRating_RegisterRating*, reg_rating, rating->register_ratings)
    ConfigRating_RegisterRatingFree(reg_rating);

  for_each_array(ConfigRating_MethodRating*, met_rating, rating->method_ratings)
    ConfigRating_MethodRatingFree(met_rating);

  memset(rating, 0, sizeof(*rating));
}

const char* RegisterType_ToStr(enum RegisterType type) {
  switch (type) {
    case RegisterType_FanReadRegister:
      return "FanReadRegister";
    case RegisterType_FanWriteRegister:
      return "FanWriteRegister";
    case RegisterType_RegisterWriteConfigurationRegister:
      return "RegisterWriteConfigurationRegister";
  }

  return "?";
}

const char* RegisterScore_ToStr(enum RegisterScore score) {
  switch (score) {
    case RegisterScore_FullMatch:
      return "FullMatch";
    case RegisterScore_PartialMatch:
      return "PartialMatch";
    case RegisterScore_MinimalMatch:
      return "MinimalMatch";
    case RegisterScore_NoMatch:
      return "NoMatch";
    case RegisterScore_NotFound:
      return "NotFound";
    case RegisterScore_BadRegister:
      return "BadRegister";
  }

  return "?";
}

const char* MethodScore_ToStr(enum MethodScore score) {
  switch (score) {
    case MethodScore_Found:
      return "Found";
    case MethodScore_NotFound:
      return "NotFound";
  }

  return "?";
}

static nx_json* RegisterRating_ToJson(ConfigRating_RegisterRating* rating, const char* key, nx_json* parent) {
  nx_json* object = create_json_object(key, parent);

  create_json_integer("offset", object, rating->offset);
  create_json_string("type", object, RegisterType_ToStr(rating->type));
  create_json_string("score", object, RegisterScore_ToStr(rating->score));

  if (rating->score != RegisterScore_NotFound) {
    AcpiRegister_ToJson(rating->info, "info", object);
  }

  return object;
}

static nx_json* MethodRating_ToJson(ConfigRating_MethodRating* rating, const char* key, nx_json* parent) {
  nx_json* object = create_json_object(key, parent);

  create_json_string("call", object, rating->call);
  create_json_string("score", object, MethodScore_ToStr(rating->score));

  if (rating->score != MethodScore_NotFound) {
    AcpiMethod_ToJson(rating->info, "info", object);
  }

  return object;
}

nx_json* ConfigRating_ToJson(ConfigRating_Rating* rating, const char* key, nx_json* parent) {
  nx_json* object = create_json_object(key, parent);

  create_json_double("score", object, rating->score);
  create_json_integer("priority", object, rating->priority);

  nx_json* register_ratings = create_json_array("register_ratings", object);
  for_each_array(ConfigRating_RegisterRating*, r, rating->register_ratings)
    RegisterRating_ToJson(r, NULL, register_ratings);

  nx_json* method_ratings = create_json_array("method_ratings", object);
  for_each_array(ConfigRating_MethodRating*, r, rating->method_ratings)
    MethodRating_ToJson(r, NULL, method_ratings);

  return object;
}
