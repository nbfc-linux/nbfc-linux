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

  if (cfg->isset.UpThreshold) {
    create_json_integer("UpThreshold", obj, cfg->UpThreshold);
  }

  if (cfg->isset.DownThreshold) {
    create_json_integer("DownThreshold", obj, cfg->DownThreshold);
  }

  if (cfg->isset.FanSpeed) {
    create_json_double("FanSpeed", obj, cfg->FanSpeed);
  }
}

static void FanSpeedPercentageOverride_ToJson(const FanSpeedPercentageOverride* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (cfg->isset.FanSpeedPercentage) {
    create_json_double("FanSpeedPercentage", obj, cfg->FanSpeedPercentage);
  }

  if (cfg->isset.FanSpeedValue) {
    create_json_integer("FanSpeedValue", obj, cfg->FanSpeedValue);
  }

  if (cfg->isset.TargetOperation) {
    const char* str = "?";
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

  if (cfg->isset.WriteOccasion) {
    const char* str = "?";
    switch (cfg->WriteOccasion) {
    case RegisterWriteOccasion_OnInitialization: str = "OnInitialization"; break;
    case RegisterWriteOccasion_OnWriteFanSpeed:  str = "OnWriteFanSpeed";  break;
    case RegisterWriteOccasion_Unset: break;
    }
    create_json_string("WriteOccasion", obj, str);
  }

  if (cfg->isset.WriteMode) {
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

  if (cfg->isset.Register) {
    create_json_integer("Register", obj, cfg->Register);
  }

  if (cfg->isset.Value) {
    create_json_integer("Value", obj, cfg->Value);
  }

  if (cfg->isset.AcpiMethod) {
    create_json_string("AcpiMethod", obj, cfg->AcpiMethod);
  }

  if (cfg->isset.LuaCode) {
    LuaCode_ToJson(cfg->LuaCode.source, obj, "LuaCode");
  }

  if (cfg->isset.ResetRequired) {
    create_json_bool("ResetRequired", obj, cfg->ResetRequired);
  }

  if (cfg->isset.ResetWriteMode) {
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

  if (cfg->isset.ResetValue) {
    create_json_integer("ResetValue", obj, cfg->ResetValue);
  }

  if (cfg->isset.ResetAcpiMethod) {
    create_json_string("ResetAcpiMethod", obj, cfg->ResetAcpiMethod);
  }

  if (cfg->isset.ResetLuaCode) {
    LuaCode_ToJson(cfg->ResetLuaCode.source, obj, "ResetLuaCode");
  }

  if (cfg->isset.Description) {
    create_json_string("Description", obj, cfg->Description);
  }
}

static void FanConfiguration_ToJson(const FanConfiguration* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (cfg->isset.FanDisplayName) {
    create_json_string("FanDisplayName", obj, cfg->FanDisplayName);
  }

  if (cfg->isset.ReadRegister) {
    create_json_integer("ReadRegister", obj, cfg->ReadRegister);
  }

  if (cfg->isset.ReadAcpiMethod) {
    create_json_string("ReadAcpiMethod", obj, cfg->ReadAcpiMethod);
  }

  if (cfg->isset.ReadLuaCode) {
    LuaCode_ToJson(cfg->ReadLuaCode.source, obj, "ReadLuaCode");
  }

  if (cfg->isset.WriteRegister) {
    create_json_integer("WriteRegister", obj, cfg->WriteRegister);
  }

  if (cfg->isset.WriteAcpiMethod) {
    create_json_string("WriteAcpiMethod", obj, cfg->WriteAcpiMethod);
  }

  if (cfg->isset.WriteLuaCode) {
    LuaCode_ToJson(cfg->WriteLuaCode.source, obj, "WriteLuaCode");
  }

  if (cfg->isset.MinSpeedValue) {
    create_json_integer("MinSpeedValue", obj, cfg->MinSpeedValue);
  }

  if (cfg->isset.MaxSpeedValue) {
    create_json_integer("MaxSpeedValue", obj, cfg->MaxSpeedValue);
  }

  if (cfg->isset.MinSpeedValueRead) {
    create_json_integer("MinSpeedValueRead", obj, cfg->MinSpeedValueRead);
  }

  if (cfg->isset.MaxSpeedValueRead) {
    create_json_integer("MaxSpeedValueRead", obj, cfg->MaxSpeedValueRead);
  }

  if (cfg->isset.IndependentReadMinMaxValues) {
    create_json_bool("IndependentReadMinMaxValues", obj, cfg->IndependentReadMinMaxValues);
  }

  if (cfg->isset.ResetRequired) {
    create_json_bool("ResetRequired", obj, cfg->ResetRequired);
  }

  if (cfg->isset.FanSpeedResetValue) {
    create_json_integer("FanSpeedResetValue", obj, cfg->FanSpeedResetValue);
  }

  if (cfg->isset.ResetAcpiMethod) {
    create_json_string("ResetAcpiMethod", obj, cfg->ResetAcpiMethod);
  }

  if (cfg->isset.ResetLuaCode) {
    LuaCode_ToJson(cfg->ResetLuaCode.source, obj, "ResetLuaCode");
  }

  if (cfg->isset.TemperatureAlgorithmType) {
    const char* str = "?";
    switch (cfg->TemperatureAlgorithmType) {
    case TemperatureAlgorithmType_Min:     str = "Min";     break;
    case TemperatureAlgorithmType_Max:     str = "Max";     break;
    case TemperatureAlgorithmType_Average: str = "Average"; break;
    case TemperatureAlgorithmType_Unset:   break;
    }
    create_json_string("TemperatureAlgorithmType", obj, str);
  }

  if (cfg->isset.Sensors) {
    nx_json* array = create_json_array("Sensors", obj);
    for_each_array(str*, sensor, cfg->Sensors)
      create_json_string(NULL, array, *sensor);
  }

  if (cfg->isset.TemperatureThresholds) {
    nx_json* array = create_json_array("TemperatureThresholds", obj);
    for_each_array(TemperatureThreshold*, tt, cfg->TemperatureThresholds)
      TemperatureThreshold_ToJson(tt, array, NULL);
  }

  if (cfg->isset.FanSpeedPercentageOverrides) {
    nx_json* array = create_json_array("FanSpeedPercentageOverrides", obj);
    for_each_array(FanSpeedPercentageOverride*, fspo, cfg->FanSpeedPercentageOverrides)
      FanSpeedPercentageOverride_ToJson(fspo, array, NULL);
  }
}

static void Sponsor_ToJson(const Sponsor* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (cfg->isset.Name) {
    create_json_string("Name", obj, cfg->Name);
  }

  if (cfg->isset.Description) {
    create_json_string("Description", obj, cfg->Description);
  }

  if (cfg->isset.URL) {
    create_json_string("URL", obj, cfg->URL);
  }

  if (cfg->isset.BannerURL) {
    create_json_string("BannerURL", obj, cfg->BannerURL);
  }
}

void ModelConfig_ToJson(const ModelConfig* cfg, nx_json* parent, const char* key) {
  nx_json* obj = create_json_object(key, parent);

  if (cfg->isset.LegacyTemperatureThresholdsBehaviour) {
    create_json_bool("LegacyTemperatureThresholdsBehaviour", obj, cfg->LegacyTemperatureThresholdsBehaviour);
  }

  if (cfg->isset.NotebookModel) {
    create_json_string("NotebookModel", obj, cfg->NotebookModel);
  }

  if (cfg->isset.Author) {
    create_json_string("Author", obj, cfg->Author);
  }

  if (cfg->isset.LuaLibraries) {
    // TODO: LuaLibraries
  }

  if (cfg->isset.EcPollInterval) {
    create_json_integer("EcPollInterval", obj, cfg->EcPollInterval);
  }

  if (cfg->isset.CriticalTemperature) {
    create_json_integer("CriticalTemperature", obj, cfg->CriticalTemperature);
  }

  if (cfg->isset.CriticalTemperatureOffset) {
    create_json_integer("CriticalTemperatureOffset", obj, cfg->CriticalTemperatureOffset);
  }

  if (cfg->isset.ReadWriteWords) {
    create_json_bool("ReadWriteWords", obj, cfg->ReadWriteWords);
  }

  if (cfg->isset.Sponsor) {
    Sponsor_ToJson(&cfg->Sponsor, obj, "Sponsor");
  }

  if (cfg->isset.FanConfigurations) {
    nx_json* array = create_json_array("FanConfigurations", obj);
    for_each_array(FanConfiguration*, fan_config, cfg->FanConfigurations)
      FanConfiguration_ToJson(fan_config, array, NULL);
  }

  if (cfg->isset.RegisterWriteConfigurations) {
    nx_json* array = create_json_array("RegisterWriteConfigurations", obj);
    for_each_array(RegisterWriteConfiguration*, rwc_config, cfg->RegisterWriteConfigurations)
      RegisterWriteConfiguration_ToJson(rwc_config, array, NULL);
  }
}

/**
 * Unset all fields with empty arrays in ModelConfig.
 */
void ModelConfig_RemoveEmptyArrays(ModelConfig* cfg) {
  if (cfg->isset.FanConfigurations &&
      cfg->FanConfigurations.size == 0)
  {
    cfg->isset.FanConfigurations = false;
  }

  if (cfg->isset.RegisterWriteConfigurations &&
      cfg->RegisterWriteConfigurations.size == 0)
  {
    cfg->isset.RegisterWriteConfigurations = false;
  }

  for_each_array(FanConfiguration*, fan_config, cfg->FanConfigurations) {
    if (fan_config->isset.Sensors &&
        fan_config->Sensors.size == 0)
    {
      fan_config->isset.Sensors = false;
    }

    if (fan_config->isset.TemperatureThresholds &&
        fan_config->TemperatureThresholds.size == 0)
    {
      fan_config->isset.TemperatureThresholds = false;
    }

    if (fan_config->isset.FanSpeedPercentageOverrides &&
        fan_config->FanSpeedPercentageOverrides.size == 0)
    {
      fan_config->isset.FanSpeedPercentageOverrides = false;
    }
  }
}

/**
 * Unset all fields with empty strings in ModelConfig.
 */
void ModelConfig_RemoveEmptyStrings(ModelConfig* cfg) {
  if (cfg->isset.Author &&
      strlen(cfg->Author) == 0)
  {
    cfg->isset.Author = false;
  }

  for_each_array(RegisterWriteConfiguration*, rwc_config, cfg->RegisterWriteConfigurations) {
    if (rwc_config->isset.Description &&
        strlen(rwc_config->Description) == 0)
    {
      rwc_config->isset.Description = false;
    }
  }

  for_each_array(FanConfiguration*, fan_config, cfg->FanConfigurations) {
    if (fan_config->isset.FanDisplayName &&
        strlen(fan_config->FanDisplayName) == 0)
    {
      fan_config->isset.FanDisplayName = false;
    }
  }
}
