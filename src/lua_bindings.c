#include "lua_bindings.h"

#include "ec.h"
#include "acpi_call.h"

#include <string.h> // strcmp
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern const EC_VTable* ec;
static uint64_t Lua_CurrentValue;
static lua_State* Lua_State = NULL;

static inline int Lua_Return_Error(lua_State* l, const char* err) {
  lua_pushstring(l, err);
  lua_pushnil(l);
  return 2;
}

static inline int Lua_Return_Result(lua_State* l, uint64_t result) {
  lua_pushnil(l);
  lua_pushinteger(l, result);
  return 2;
}

static int Lua_Get_Value(lua_State* l) {
  lua_pushinteger(l, Lua_CurrentValue);
  return 1;
}

static int Lua_EC_Read(lua_State* l) {
  Error e;
  lua_Integer register_ = luaL_checkinteger(l, 1);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_read(): register < 0");

  if (register_ > 255)
    return Lua_Return_Error(l, "ec_read(): register > 255");

  uint8_t byte;
  e = ec->ReadByte((uint8_t) register_, &byte);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Result(l, byte);
}

static int Lua_EC_ReadWord(lua_State* l) {
  Error e;
  lua_Integer register_ = luaL_checkinteger(l, 1);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_read_word(): register < 0");

  if (register_ > 254)
    return Lua_Return_Error(l, "ec_read_word(): register > 254");

  uint16_t word;
  e = ec->ReadWord((uint8_t) register_, &word);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Result(l, word);
}

static int Lua_EC_Write(lua_State* l) {
  Error e;
  lua_Integer register_ = luaL_checkinteger(l, 1);
  lua_Integer value = luaL_checkinteger(l, 2);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_write(): register < 0");

  if (register_ > 255)
    return Lua_Return_Error(l, "ec_write(): register > 255");

  if (value < 0)
    return Lua_Return_Error(l, "ec_write(): value < 0");

  if (value > 255)
    return Lua_Return_Error(l, "ec_write(): value > 255");

  e = ec->WriteByte((uint8_t) register_, (uint8_t) value);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Result(l, 0);
}

static int Lua_EC_WriteWord(lua_State* l) {
  Error e;
  lua_Integer register_ = luaL_checkinteger(l, 1);
  lua_Integer value = luaL_checkinteger(l, 2);

  if (register_ < 0)
    return Lua_Return_Error(l, "ec_write_word(): register < 0");

  if (register_ > 254)
    return Lua_Return_Error(l, "ec_write_word(): register > 254");

  if (value < 0)
    return Lua_Return_Error(l, "ec_write_word(): value < 0");

  if (value > UINT16_MAX)
    return Lua_Return_Error(l, "ec_write_word(): value > 65535");

  e = ec->WriteWord((uint8_t) register_, (uint16_t) value);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Result(l, 0);
}

static int Lua_ACPI_Call(lua_State* l) {
  Error e;
  size_t method_len;
  const char* method = luaL_checklstring(l, 1, &method_len);
  uint64_t result;

  e = AcpiCall_Open();
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  e = AcpiCall_Call(method, 0, &result);
  if (e)
    return Lua_Return_Error(l, err_print_all(e));

  return Lua_Return_Result(l, result);
}

static Error Lua_Open(void) {
  if (Lua_State)
    return err_success();

  Lua_State = luaL_newstate();
  if (! Lua_State)
    return err_string("luaL_newstate() failed");

  lua_register(Lua_State, "get_value",     Lua_Get_Value);
  lua_register(Lua_State, "ec_read",       Lua_EC_Read);
  lua_register(Lua_State, "ec_read_word",  Lua_EC_ReadWord);
  lua_register(Lua_State, "ec_write",      Lua_EC_Write);
  lua_register(Lua_State, "ec_write_word", Lua_EC_WriteWord);
  lua_register(Lua_State, "acpi_call",     Lua_ACPI_Call);
  return err_success();
}

Error Lua_UseLibrary(const char* libname) {
  Error e = Lua_Open();
  if (e)
    return e;

  if (! strcmp(libname, "base"))
    luaL_requiref(Lua_State, "_G", luaopen_base, 1);
  else if (! strcmp(libname, LUA_MATHLIBNAME))
    luaL_requiref(Lua_State, LUA_MATHLIBNAME, luaopen_math, 1);
  else if (! strcmp(libname, LUA_STRLIBNAME))
    luaL_requiref(Lua_State, LUA_STRLIBNAME, luaopen_string, 1);
  else if (! strcmp(libname, LUA_TABLIBNAME))
    luaL_requiref(Lua_State, LUA_TABLIBNAME, luaopen_table, 1);
  else if (! strcmp(libname, LUA_IOLIBNAME))
    luaL_requiref(Lua_State, LUA_IOLIBNAME, luaopen_io, 1);
  else if (! strcmp(libname, LUA_OSLIBNAME))
    luaL_requiref(Lua_State, LUA_OSLIBNAME, luaopen_os, 1);
  else
    return err_stringf("%s: Unkown Lua library", libname);

  lua_pop(Lua_State, 1);
  return err_success();
}

Error Lua_LoadCode(const char* code, int* function_ref) {
  Error e;

  e = Lua_Open();
  if (e)
    return e;

  if (luaL_loadstring(Lua_State, code) != LUA_OK)
    return err_string(lua_tostring(Lua_State, -1));

  *function_ref = luaL_ref(Lua_State, LUA_REGISTRYINDEX);
  return err_success();
}

Error Lua_Call(int function_ref, uint64_t value, uint64_t* result) {
  Error e;
  Lua_CurrentValue = value;

  // push function
  lua_rawgeti(Lua_State, LUA_REGISTRYINDEX, function_ref);

  // check if it's really a function
  if (! lua_isfunction(Lua_State, -1)) {
    e = err_string("lua_rawgeti() did not return a function");
    lua_pop(Lua_State, 1);
    return e;
  }

  // call
  if (lua_pcall(Lua_State, 0, LUA_MULTRET, 0) != LUA_OK) {
    e = err_string(lua_tostring(Lua_State, -1));
    lua_pop(Lua_State, 1);
    return e;
  }

  // ensure we actually have 2 return values
  int top = lua_gettop(Lua_State);
  if (top != 2) {
    lua_pop(Lua_State, top);
    e = err_string("Lua code did not return 2 values");
    return e;
  }

  // stack:
  // -2 = error
  // -1 = result
  
  // check error
  if (! lua_isnil(Lua_State, -2)) {
    const char* err = lua_tostring(Lua_State, -2);
    e = err_string(err ? err : "Unkown Lua error");
    lua_pop(Lua_State, 2);
    return e;
  }

  // check result
  if (! lua_isinteger(Lua_State, -1)) {
    lua_pop(Lua_State, 2);
    e = err_string("Result is not an integer");
    return e;
  }

  *result = (uint64_t) lua_tointeger(Lua_State, -1);

  lua_pop(Lua_State, 2);
  return err_success();
}

void Lua_Close(void) {
  if (Lua_State) {
    lua_close(Lua_State);
    Lua_State = NULL;
  }
}
