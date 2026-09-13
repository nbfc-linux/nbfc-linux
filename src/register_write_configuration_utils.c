#include "register_write_configuration_utils.h"

#include "ec.h"
#include "acpi_call.h"
#include "lua_bindings.h"

Error RegisterWriteConfiguration_Apply(RegisterWriteConfiguration* cfg) {
  Error e;
  uint8_t mask;
  uint64_t out;

  switch (cfg->WriteMode) {
  case RegisterWriteMode_Set:
    return ec->WriteByte(cfg->Register, cfg->Value);

  case RegisterWriteMode_And:
    e = ec->ReadByte(cfg->Register, &mask);
    e_check();
    return ec->WriteByte(cfg->Register, cfg->Value & mask);

  case RegisterWriteMode_Or:
    e = ec->ReadByte(cfg->Register, &mask);
    e_check();
    return ec->WriteByte(cfg->Register, cfg->Value | mask);

  case RegisterWriteMode_Call:
    e = AcpiCall_Call(cfg->AcpiMethod, 0, &out);
    if (e)
      return err_chain_string(e, "AcpiMethod");
    else
      return err_success();

  case RegisterWriteMode_Lua:
    e = Lua_Call(cfg->LuaCode.function, 0, &out);
    if (e)
      return err_chain_string(e, "LuaCode");
    else
      return err_success();

  default:
    return err_string("ERR-01");
  }
}

Error RegisterWriteConfigurations_Apply(array_of(RegisterWriteConfiguration)* rwcs, bool initializing) {
  for_each_array(RegisterWriteConfiguration*, cfg, *rwcs) {
    if (initializing || cfg->WriteOccasion == RegisterWriteOccasion_OnWriteFanSpeed) {
       Error e = RegisterWriteConfiguration_Apply(cfg);
       if (e)
         return e;
    }
  }
  return err_success();
}

Error RegisterWriteConfiguration_Reset(RegisterWriteConfiguration* cfg) {
  Error e;
  uint8_t mask;
  uint64_t out;

  switch (cfg->ResetWriteMode) {
  case RegisterWriteMode_Set:
    return ec->WriteByte(cfg->Register, cfg->ResetValue);

  case RegisterWriteMode_And:
    e = ec->ReadByte(cfg->Register, &mask);
    e_check();
    return ec->WriteByte(cfg->Register, cfg->ResetValue & mask);

  case RegisterWriteMode_Or:
    e = ec->ReadByte(cfg->Register, &mask);
    e_check();
    return ec->WriteByte(cfg->Register, cfg->ResetValue | mask);

  case RegisterWriteMode_Call:
    e = AcpiCall_Call(cfg->ResetAcpiMethod, 0, &out);
    if (e)
      return err_chain_string(e, "ResetAcpiMethod");
    else
      return err_success();

  case RegisterWriteMode_Lua:
    e = Lua_Call(cfg->ResetLuaCode.function, 0, &out);
    if (e)
      return err_chain_string(e, "ResetLuaCode");
    else
      return err_success();

  default:
    return err_string("ERR-02");
  }
}

Error RegisterWriteConfigurations_Reset(array_of(RegisterWriteConfiguration)* rwcs) {
  Error e = err_success();

  for_each_array(RegisterWriteConfiguration*, cfg, *rwcs) {
    if (cfg->ResetRequired) {
      e = RegisterWriteConfiguration_Reset(cfg);
      e_warn();
    }
  }

  return e;
}
