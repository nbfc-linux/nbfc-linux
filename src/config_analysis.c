#include "config_analysis.h"

#include "acpi_analysis.h"
#include "lua_bindings.h"

#include <string.h> // memset
#include <stdio.h>  // snprintf

/*
 * The value returned by our dummy functions:
 * - ec_read()
 * - ec_read_word()
 * - acpi_call()
 * - acpi_call_raw()
 * - acpi_get_int()
 */
#define CONFIG_ANALYSIS_DUMMY_RETURN_VALUE     1
#define CONFIG_ANALYSIS_DUMMY_RETURN_VALUE_RAW "1"

static ConfigAnalysis ConfigAnalysis_Info;
static bool ConfigAnalysis_InRegisterWriteConfiguration;

/**
 * Push a register into `ConfigAnalysis_Info` if it does not already exist.
 *
 * The register is only added if no existing entry with the same register
 * address and RegisterType is present.
 *
 * Returns an error if the maximum number of registers is exceeded.
 */
static Error ConfigAnalysis_PushRegister(
  uint8_t register_,
  enum RegisterType type,
  bool word
) {
  for (size_t i = 0; i < ConfigAnalysis_Info.registers_size; ++i) {
    if (ConfigAnalysis_Info.registers[i].register_ == register_ &&
        ConfigAnalysis_Info.registers[i].type == type)
    {
      return err_success();
    }
  }

  if (ConfigAnalysis_Info.registers_size >= CONFIG_ANALYSIS_MAX_REGISTERS)
    return err_string("Too many registers found in model configuration");

  const size_t idx = ConfigAnalysis_Info.registers_size;
  ConfigAnalysis_Info.registers[idx].register_ = register_;
  ConfigAnalysis_Info.registers[idx].type = type;
  ConfigAnalysis_Info.registers[idx].word = word;
  ConfigAnalysis_Info.registers_size++;
  return err_success();
}

/**
 * Push a method call into `ConfigAnalysis_Info`.
 *
 * The full method string, including its arguments, is stored.
 *
 * If `unique` is true, the method is only added if no existing entry with
 * the same method name is found.
 *
 * Returns an error if the maximum number of methods is exceeded.
 */
static Error ConfigAnalysis_PushMethod(const char* method, bool unique) {
  while (*method && *method <= 32)
    ++method; // Skip whitespace

  if (! *method)
    return err_string("ACPI method is empty");

  if (unique) {
    for (size_t i = 0; i < ConfigAnalysis_Info.methods_size; ++i)
      if (Acpi_Analysis_Path_Equals(method, ConfigAnalysis_Info.methods[i]))
        return err_success();
  }

  if (ConfigAnalysis_Info.methods_size >= CONFIG_ANALYSIS_MAX_METHODS)
    return err_string("Too many ACPI methods found in model configuration");

  snprintf(ConfigAnalysis_Info.methods[ConfigAnalysis_Info.methods_size++],
    CONFIG_ANALYSIS_MAX_METHOD_LEN + 1, "%s", method);

  return err_success();
}

static int ConfigAnalysis_EC_Read(lua_State* l) {
  const lua_Integer register_ = luaL_checkinteger(l, 1);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_read(): register < 0");

  if (register_ > 255)
    return Lua_Return_Error(l, "ec_read(): register > 255");

  const enum RegisterType type = (
    ConfigAnalysis_InRegisterWriteConfiguration ?
    RegisterType_RegisterWriteConfigurationRegister :
    RegisterType_FanReadRegister
  );

  Error e = ConfigAnalysis_PushRegister((uint8_t) register_, type, false);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Integer(l, CONFIG_ANALYSIS_DUMMY_RETURN_VALUE);
}

static int ConfigAnalysis_EC_ReadWord(lua_State* l) {
  const lua_Integer register_ = luaL_checkinteger(l, 1);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_read_word(): register < 0");

  if (register_ > 254)
    return Lua_Return_Error(l, "ec_read_word(): register > 254");

  const enum RegisterType type = (
    ConfigAnalysis_InRegisterWriteConfiguration ?
    RegisterType_RegisterWriteConfigurationRegister :
    RegisterType_FanReadRegister
  );

  Error e = ConfigAnalysis_PushRegister((uint8_t) register_, type, true);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Integer(l, CONFIG_ANALYSIS_DUMMY_RETURN_VALUE);
}

static int ConfigAnalysis_EC_Write(lua_State* l) {
  const lua_Integer register_ = luaL_checkinteger(l, 1);
  const lua_Integer value = luaL_checkinteger(l, 2);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_write(): register < 0");

  if (register_ > 255)
    return Lua_Return_Error(l, "ec_write(): register > 255");

  if (value < 0)
    return Lua_Return_Error(l, "ec_write(): value < 0");

  if (value > 255)
    return Lua_Return_Error(l, "ec_write(): value > 255");

  const enum RegisterType type = (
    ConfigAnalysis_InRegisterWriteConfiguration ?
    RegisterType_RegisterWriteConfigurationRegister :
    RegisterType_FanWriteRegister
  );

  Error e = ConfigAnalysis_PushRegister((uint8_t) register_, type, false);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Integer(l, 0);
}

static int ConfigAnalysis_EC_WriteWord(lua_State* l) {
  const lua_Integer register_ = luaL_checkinteger(l, 1);
  const lua_Integer value = luaL_checkinteger(l, 2);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_write_word(): register < 0");

  if (register_ > 254)
    return Lua_Return_Error(l, "ec_write_word(): register > 254");

  if (value < 0)
    return Lua_Return_Error(l, "ec_write_word(): value < 0");

  if (value > UINT16_MAX)
    return Lua_Return_Error(l, "ec_write_word(): value > 65535");

  const enum RegisterType type = (
    ConfigAnalysis_InRegisterWriteConfiguration ?
    RegisterType_RegisterWriteConfigurationRegister :
    RegisterType_FanWriteRegister
  );

  Error e = ConfigAnalysis_PushRegister((uint8_t) register_, type, true);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Integer(l, 0);
}

static int ConfigAnalysis_ACPI_Call(lua_State* l) {
  size_t method_len;
  const char* const method = luaL_checklstring(l, 1, &method_len);

  Error e = ConfigAnalysis_PushMethod(method, true);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Integer(l, CONFIG_ANALYSIS_DUMMY_RETURN_VALUE);
}

static int ConfigAnalysis_ACPI_CallRaw(lua_State* l) {
  size_t method_len;
  const char* const method = luaL_checklstring(l, 1, &method_len);

  Error e = ConfigAnalysis_PushMethod(method, true);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_String(l, CONFIG_ANALYSIS_DUMMY_RETURN_VALUE_RAW);
}

static int ConfigAnalysis_ACPI_GetInt(lua_State* l) {
  size_t len;
  luaL_checklstring(l, 1, &len); // result
  luaL_checklstring(l, 2, &len); // path

  return Lua_Return_Integer(l, CONFIG_ANALYSIS_DUMMY_RETURN_VALUE);
}

static Error ConfigAnalysis_Begin(void) {
  Error e;

  memset(&ConfigAnalysis_Info, 0, sizeof(ConfigAnalysis));

  e = Lua_Open();
  if (e)
    return e;

  lua_register(Lua_State, "ec_read",       ConfigAnalysis_EC_Read);
  lua_register(Lua_State, "ec_read_word",  ConfigAnalysis_EC_ReadWord);
  lua_register(Lua_State, "ec_write",      ConfigAnalysis_EC_Write);
  lua_register(Lua_State, "ec_write_word", ConfigAnalysis_EC_WriteWord);
  lua_register(Lua_State, "acpi_call",     ConfigAnalysis_ACPI_Call);
  lua_register(Lua_State, "acpi_call_raw", ConfigAnalysis_ACPI_CallRaw);
  lua_register(Lua_State, "acpi_get_int",  ConfigAnalysis_ACPI_GetInt);
  return err_success();
}

Error ConfigAnalysis_AnalyzeFanConfiguration(
  FanConfiguration* fan_config,
  ModelConfig* model_config
) {
  Error e;
  uint64_t dummy;

  // Read operations
  if (FanConfiguration_IsSet_ReadRegister(fan_config)) {
    e = ConfigAnalysis_PushRegister(
      fan_config->ReadRegister,
      RegisterType_FanReadRegister,
      model_config->ReadWriteWords);
    if (e) return e;
  }

  if (FanConfiguration_IsSet_ReadAcpiMethod(fan_config)) {
    e = ConfigAnalysis_PushMethod(fan_config->ReadAcpiMethod, false);
    if (e) return e;
  }

  if (FanConfiguration_IsSet_ReadLuaCode(fan_config)) {
    e = Lua_Call(fan_config->ReadLuaCode.function, 0, &dummy);
    if (e) return e;
  }

  // Write operations
  if (FanConfiguration_IsSet_WriteRegister(fan_config)) {
    e = ConfigAnalysis_PushRegister(
      fan_config->WriteRegister,
      RegisterType_FanWriteRegister,
      model_config->ReadWriteWords);
    if (e) return e;
  }

  if (FanConfiguration_IsSet_WriteAcpiMethod(fan_config)) {
    e = ConfigAnalysis_PushMethod(fan_config->WriteAcpiMethod, false);
    if (e) return e;
  }

  if (FanConfiguration_IsSet_WriteLuaCode(fan_config)) {
    if (fan_config->MinSpeedValue < fan_config->MaxSpeedValue) {
      for (uint64_t i = fan_config->MinSpeedValue; i <= fan_config->MaxSpeedValue; ++i) {
        e = Lua_Call(fan_config->WriteLuaCode.function, i, &dummy);
        if (e) return e;
      }
    }
    else {
      for (uint64_t i = fan_config->MaxSpeedValue; i <= fan_config->MinSpeedValue; ++i) {
        e = Lua_Call(fan_config->WriteLuaCode.function, i, &dummy);
        if (e) return e;
      }
    }
  }

  // Reset operations
  if (FanConfiguration_IsSet_ResetAcpiMethod(fan_config)) {
    e = ConfigAnalysis_PushMethod(fan_config->ResetAcpiMethod, false);
    if (e) return e;
  }

  if (FanConfiguration_IsSet_ResetLuaCode(fan_config)) {
    e = Lua_Call(fan_config->ResetLuaCode.function, 0, &dummy);
    if (e) return e;
  }

  return err_success();
}

Error ConfigAnalysis_AnalyzeModelConfig(
  ModelConfig* model_config,
  ConfigAnalysis** out
) {
  Error e;

  e = ConfigAnalysis_Begin();
  if (e) return e;

  ConfigAnalysis_InRegisterWriteConfiguration = false;
  for_each_array(FanConfiguration*, fan_config, model_config->FanConfigurations) {
    e = ConfigAnalysis_AnalyzeFanConfiguration(fan_config, model_config);
    if (e) return e;
  }

  ConfigAnalysis_InRegisterWriteConfiguration = true;
  for_each_array(RegisterWriteConfiguration*, rw_config, model_config->RegisterWriteConfigurations) {
    // Set operations
    if (RegisterWriteConfiguration_IsSet_Register(rw_config)) {
      e = ConfigAnalysis_PushRegister(rw_config->Register, RegisterType_RegisterWriteConfigurationRegister, false);
      if (e) return e;
    }

    if (RegisterWriteConfiguration_IsSet_AcpiMethod(rw_config)) {
      e = ConfigAnalysis_PushMethod(rw_config->AcpiMethod, false);
      if (e) return e;
    }

    if (RegisterWriteConfiguration_IsSet_LuaCode(rw_config)) {
      uint64_t dummy;
      e = Lua_Call(rw_config->LuaCode.function, 0, &dummy);
      if (e) return e;
    }

    // Reset operations
    if (RegisterWriteConfiguration_IsSet_ResetAcpiMethod(rw_config)) {
      e = ConfigAnalysis_PushMethod(rw_config->ResetAcpiMethod, false);
      if (e) return e;
    }

    if (RegisterWriteConfiguration_IsSet_ResetLuaCode(rw_config)) {
      uint64_t dummy;
      e = Lua_Call(rw_config->ResetLuaCode.function, 0, &dummy);
      if (e) return e;
    }
  }

  *out = &ConfigAnalysis_Info;
  return err_success();
}
