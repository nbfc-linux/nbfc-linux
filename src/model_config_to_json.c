#include "model_config_to_json.h"

#include "nxjson_utils.h"

static void LuaCode_ToJson(const char* code, nx_json* parent, const char* key) {
  size_t len = strlen(code);
  if (! len)
    return;

  char* copy = Mem_Strdup(code);

  // Trim last newline
  if (copy[len - 1] == '\n')
    copy[len - 1] = '\0';

  char* newline = strchr(copy, '\n');

  if (! newline)
    create_json_string(key, parent, copy);
  else {
    nx_json* array = create_json_array(key, parent);
    char* s = copy;

    while (s) {
      newline = strchr(s, '\n');
      if (newline)
        *newline = '\0';
      create_json_string(NULL, array, Mem_Strdup(s));
      s = newline ? newline + 1 : NULL;
    }

    Mem_Free(copy);
  }
}

static void TemperatureThreshold_ToJson(const TemperatureThreshold* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (TemperatureThreshold_IsSet_UpThreshold(cfg)) {
    create_json_integer("UpThreshold", obj, cfg->UpThreshold);
  }

  if (TemperatureThreshold_IsSet_UpThreshold(cfg)) {
    create_json_integer("DownThreshold", obj, cfg->DownThreshold);
  }

  if (TemperatureThreshold_IsSet_FanSpeed(cfg)) {
    create_json_double("FanSpeed", obj, cfg->FanSpeed);
  }
}

static void FanSpeedPercentageOverride_ToJson(const FanSpeedPercentageOverride* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (FanSpeedPercentageOverride_IsSet_FanSpeedPercentage(cfg)) {
    create_json_double("FanSpeedPercentage", obj, cfg->FanSpeedPercentage);
  }

  if (FanSpeedPercentageOverride_IsSet_FanSpeedValue(cfg)) {
    create_json_integer("FanSpeedValue", obj, cfg->FanSpeedValue);
  }

  if (FanSpeedPercentageOverride_IsSet_TargetOperation(cfg)) {
    const char *str = "?";
    switch (cfg->TargetOperation) {
    case OverrideTargetOperation_Read:      str = "Read";      break;
    case OverrideTargetOperation_Write:     str = "Write";     break;
    case OverrideTargetOperation_ReadWrite: str = "ReadWrite"; break;
    case OverrideTargetOperation_Unset:     break;
    }
    create_json_string("TargetOperation", obj, str);
  }
}

static void RegisterWriteConfiguration_ToJson(const RegisterWriteConfiguration* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (RegisterWriteConfiguration_IsSet_WriteOccasion(cfg)) {
    const char *str = "?";
    switch (cfg->WriteOccasion) {
    case RegisterWriteOccasion_OnInitialization: str = "OnInitialization"; break;
    case RegisterWriteOccasion_OnWriteFanSpeed:  str = "OnWriteFanSpeed";  break;
    case RegisterWriteOccasion_Unset: break;
    }
    create_json_string("WriteOccasion", obj, str);
  }

  if (RegisterWriteConfiguration_IsSet_WriteMode(cfg)) {
    const char* str = "?";
    switch (cfg->WriteMode) {
    case RegisterWriteMode_Set:   str = "Set";  break;
    case RegisterWriteMode_And:   str = "And";  break;
    case RegisterWriteMode_Or:    str = "Or";   break;
    case RegisterWriteMode_Call:  str = "Call"; break;
    case RegisterWriteMode_Lua:   str = "Lua";  break;
    case RegisterWriteMode_Unset: break;
    }
    create_json_string("WriteMode", obj, str);
  }

  if (RegisterWriteConfiguration_IsSet_Register(cfg)) {
    create_json_integer("Register", obj, cfg->Register);
  }

  if (RegisterWriteConfiguration_IsSet_Value(cfg)) {
    create_json_integer("Value", obj, cfg->Value);
  }

  if (RegisterWriteConfiguration_IsSet_AcpiMethod(cfg)) {
    create_json_string("AcpiMethod", obj, cfg->AcpiMethod);
  }

  if (RegisterWriteConfiguration_IsSet_LuaCode(cfg)) {
    LuaCode_ToJson(cfg->LuaCode.source, obj, "LuaCode");
  }

  if (RegisterWriteConfiguration_IsSet_ResetRequired(cfg)) {
    create_json_bool("ResetRequired", obj, cfg->ResetRequired);
  }

  if (RegisterWriteConfiguration_IsSet_ResetWriteMode(cfg)) {
    const char* str = "?";
    switch (cfg->ResetWriteMode) {
    case RegisterWriteMode_Set:   str = "Set";  break;
    case RegisterWriteMode_And:   str = "And";  break;
    case RegisterWriteMode_Or:    str = "Or";   break;
    case RegisterWriteMode_Call:  str = "Call"; break;
    case RegisterWriteMode_Lua:   str = "Lua";  break;
    case RegisterWriteMode_Unset: break;
    }
    create_json_string("ResetWriteMode", obj, str);
  }

  if (RegisterWriteConfiguration_IsSet_ResetValue(cfg)) {
    create_json_integer("ResetValue", obj, cfg->ResetValue);
  }

  if (RegisterWriteConfiguration_IsSet_ResetAcpiMethod(cfg)) {
    create_json_string("ResetAcpiMethod", obj, cfg->ResetAcpiMethod);
  }

  if (RegisterWriteConfiguration_IsSet_ResetLuaCode(cfg)) {
    LuaCode_ToJson(cfg->ResetLuaCode.source, obj, "ResetLuaCode");
  }

  if (RegisterWriteConfiguration_IsSet_Description(cfg)) {
    create_json_string("Description", obj, cfg->Description);
  }
}

static void FanConfiguration_ToJson(const FanConfiguration* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (FanConfiguration_IsSet_FanDisplayName(cfg)) {
    create_json_string("FanDisplayName", obj, cfg->FanDisplayName);
  }

  if (FanConfiguration_IsSet_ReadRegister(cfg)) {
    create_json_integer("ReadRegister", obj, cfg->ReadRegister);
  }

  if (FanConfiguration_IsSet_ReadAcpiMethod(cfg)) {
    create_json_string("ReadAcpiMethod", obj, cfg->ReadAcpiMethod);
  }

  if (FanConfiguration_IsSet_ReadLuaCode(cfg)) {
    LuaCode_ToJson(cfg->ReadLuaCode.source, obj, "ReadLuaCode");
  }

  if (FanConfiguration_IsSet_WriteRegister(cfg)) {
    create_json_integer("WriteRegister", obj, cfg->WriteRegister);
  }

  if (FanConfiguration_IsSet_WriteAcpiMethod(cfg)) {
    create_json_string("WriteAcpiMethod", obj, cfg->WriteAcpiMethod);
  }

  if (FanConfiguration_IsSet_WriteLuaCode(cfg)) {
    LuaCode_ToJson(cfg->WriteLuaCode.source, obj, "WriteLuaCode");
  }

  if (FanConfiguration_IsSet_MinSpeedValue(cfg)) {
    create_json_integer("MinSpeedValue", obj, cfg->MinSpeedValue);
  }

  if (FanConfiguration_IsSet_MaxSpeedValue(cfg)) {
    create_json_integer("MaxSpeedValue", obj, cfg->MaxSpeedValue);
  }

  if (FanConfiguration_IsSet_MinSpeedValueRead(cfg)) {
    create_json_integer("MinSpeedValueRead", obj, cfg->MinSpeedValueRead);
  }

  if (FanConfiguration_IsSet_MaxSpeedValueRead(cfg)) {
    create_json_integer("MaxSpeedValueRead", obj, cfg->MaxSpeedValueRead);
  }

  if (FanConfiguration_IsSet_IndependentReadMinMaxValues(cfg)) {
    create_json_bool("IndependentReadMinMaxValues", obj, cfg->IndependentReadMinMaxValues);
  }

  if (FanConfiguration_IsSet_ResetRequired(cfg)) {
    create_json_bool("ResetRequired", obj, cfg->ResetRequired);
  }

  if (FanConfiguration_IsSet_FanSpeedResetValue(cfg)) {
    create_json_integer("FanSpeedResetValue", obj, cfg->FanSpeedResetValue);
  }

  if (FanConfiguration_IsSet_ResetAcpiMethod(cfg)) {
    create_json_string("ResetAcpiMethod", obj, cfg->ResetAcpiMethod);
  }

  if (FanConfiguration_IsSet_ResetLuaCode(cfg)) {
    LuaCode_ToJson(cfg->ResetLuaCode.source, obj, "ResetLuaCode");
  }

  if (FanConfiguration_IsSet_TemperatureAlgorithmType(cfg)) {
    const char* str = "?";
    switch (cfg->TemperatureAlgorithmType) {
    case TemperatureAlgorithmType_Min:     str = "Min";     break;
    case TemperatureAlgorithmType_Max:     str = "Max";     break;
    case TemperatureAlgorithmType_Average: str = "Average"; break;
    case TemperatureAlgorithmType_Unset:   break;
    }
    create_json_string("TemperatureAlgorithmType", obj, str);
  }

  if (FanConfiguration_IsSet_Sensors(cfg)) {
    nx_json* array = create_json_array("Sensors", obj);
    for_each_array(str*, sensor, cfg->Sensors)
      create_json_string(NULL, array, *sensor);
  }

  if (FanConfiguration_IsSet_TemperatureThresholds(cfg)) {
    nx_json* array = create_json_array("TemperatureThresholds", obj);
    for_each_array(TemperatureThreshold*, tt, cfg->TemperatureThresholds)
      TemperatureThreshold_ToJson(tt, array, NULL);
  }

  if (FanConfiguration_IsSet_FanSpeedPercentageOverrides(cfg)) {
    nx_json* array = create_json_array("FanSpeedPercentageOverrides", obj);
    for_each_array(FanSpeedPercentageOverride*, fspo, cfg->FanSpeedPercentageOverrides)
      FanSpeedPercentageOverride_ToJson(fspo, array, NULL);
  }
}

static void Sponsor_ToJson(const Sponsor* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (Sponsor_IsSet_Name(cfg)) {
    create_json_string("Name", obj, cfg->Name);
  }

  if (Sponsor_IsSet_Description(cfg)) {
    create_json_string("Description", obj, cfg->Description);
  }

  if (Sponsor_IsSet_URL(cfg)) {
    create_json_string("URL", obj, cfg->URL);
  }

  if (Sponsor_IsSet_BannerURL(cfg)) {
    create_json_string("BannerURL", obj, cfg->BannerURL);
  }
}

void ModelConfig_ToJson(const ModelConfig* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (ModelConfig_IsSet_LegacyTemperatureThresholdsBehaviour(cfg)) {
    create_json_bool("LegacyTemperatureThresholdsBehaviour", obj, cfg->LegacyTemperatureThresholdsBehaviour);
  }

  if (ModelConfig_IsSet_NotebookModel(cfg)) {
    create_json_string("NotebookModel", obj, cfg->NotebookModel);
  }

  if (ModelConfig_IsSet_Author(cfg)) {
    create_json_string("Author", obj, cfg->Author);
  }

  if (ModelConfig_IsSet_LuaLibraries(cfg)) {
    // TODO: LuaLibraries
  }

  if (ModelConfig_IsSet_EcPollInterval(cfg)) {
    create_json_integer("EcPollInterval", obj, cfg->EcPollInterval);
  }

  if (ModelConfig_IsSet_CriticalTemperature(cfg)) {
    create_json_integer("CriticalTemperature", obj, cfg->CriticalTemperature);
  }

  if (ModelConfig_IsSet_CriticalTemperatureOffset(cfg)) {
    create_json_integer("CriticalTemperatureOffset", obj, cfg->CriticalTemperatureOffset);
  }

  if (ModelConfig_IsSet_ReadWriteWords(cfg)) {
    create_json_bool("ReadWriteWords", obj, cfg->ReadWriteWords);
  }

  if (ModelConfig_IsSet_Sponsor(cfg)) {
    Sponsor_ToJson(&cfg->Sponsor, obj, "Sponsor");
  }

  if (ModelConfig_IsSet_FanConfigurations(cfg)) {
    nx_json* array = create_json_array("FanConfigurations", obj);
    for_each_array(FanConfiguration*, fan_config, cfg->FanConfigurations)
      FanConfiguration_ToJson(fan_config, array, NULL);
  }

  if (ModelConfig_IsSet_RegisterWriteConfigurations(cfg)) {
    nx_json* array = create_json_array("RegisterWriteConfigurations", obj);
    for_each_array(RegisterWriteConfiguration*, rwc_config, cfg->RegisterWriteConfigurations)
      RegisterWriteConfiguration_ToJson(rwc_config, array, NULL);
  }
}

/**
 * Unset all fields with empty arrays in ModelConfig.
 */
void ModelConfig_RemoveEmptyArrays(ModelConfig* cfg) {
  if (ModelConfig_IsSet_FanConfigurations(cfg) &&
      cfg->FanConfigurations.size == 0)
  {
    ModelConfig_UnSet_FanConfigurations(cfg);
  }

  if (ModelConfig_IsSet_RegisterWriteConfigurations(cfg) &&
      cfg->RegisterWriteConfigurations.size == 0)
  {
    ModelConfig_UnSet_RegisterWriteConfigurations(cfg);
  }

  for_each_array(FanConfiguration*, fan_config, cfg->FanConfigurations) {
    if (FanConfiguration_IsSet_Sensors(fan_config) &&
        fan_config->Sensors.size == 0)
    {
      FanConfiguration_UnSet_Sensors(fan_config);
    }

    if (FanConfiguration_IsSet_TemperatureThresholds(fan_config) &&
        fan_config->TemperatureThresholds.size == 0)
    {
      FanConfiguration_UnSet_TemperatureThresholds(fan_config);
    }

    if (FanConfiguration_IsSet_FanSpeedPercentageOverrides(fan_config) &&
        fan_config->FanSpeedPercentageOverrides.size == 0)
    {
      FanConfiguration_UnSet_FanSpeedPercentageOverrides(fan_config);
    }
  }
}

/**
 * Unset all fields with empty strings in ModelConfig.
 */
void ModelConfig_RemoveEmptyStrings(ModelConfig* cfg) {
  if (ModelConfig_IsSet_Author(cfg) &&
      strlen(cfg->Author) == 0)
  {
    ModelConfig_UnSet_Author(cfg);
  }

  for_each_array(RegisterWriteConfiguration*, rwc_config, cfg->RegisterWriteConfigurations) {
    if (RegisterWriteConfiguration_IsSet_Description(rwc_config) &&
        strlen(rwc_config->Description) == 0)
    {
      RegisterWriteConfiguration_UnSet_Description(rwc_config);
    }
  }

  for_each_array(FanConfiguration*, fan_config, cfg->FanConfigurations) {
    if (FanConfiguration_IsSet_FanDisplayName(fan_config) &&
        strlen(fan_config->FanDisplayName) == 0)
    {
      FanConfiguration_UnSet_FanDisplayName(fan_config);
    }
  }
}
