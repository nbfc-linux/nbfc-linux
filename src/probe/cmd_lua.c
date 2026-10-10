#include "../lua_bindings.h"
#include "../client/check_root.h"

static const struct cli99_Option Lua_CommandLine[] = {
  cli99_Options_Include(&Main_CommandLine),
  {"file", Option_File, cli99_RequiredArgument},
  cli99_Options_End()
};

static int Lua_RunFile(lua_State* L, const char* file) {
  int status = luaL_loadfile(L, file);

  if (status == LUA_OK)
    status = lua_pcall(L, 0, LUA_MULTRET, 0);

  if (status != LUA_OK) {
    const char* err = lua_tostring(L, -1);

    if (err)
      Log_Error("%s", err);
    else
      Log_Error("Lua error");

    return NBFC_EXIT_FAILURE;
  }

  return NBFC_EXIT_SUCCESS;
}

int Lua(void) {
  Error e;

  signal(SIGINT,  SIG_DFL);
  signal(SIGTERM, SIG_DFL);

  check_root();
  Initialize_EC();

  e = Lua_Open();
  e_die();

  e = Lua_UseLibrary("base");
  e_die();

  e = Lua_UseLibrary(LUA_MATHLIBNAME);
  e_die();

  e = Lua_UseLibrary(LUA_STRLIBNAME);
  e_die();

  e = Lua_UseLibrary(LUA_TABLIBNAME);
  e_die();

  e = Lua_UseLibrary(LUA_IOLIBNAME);
  e_die();

  e = Lua_UseLibrary(LUA_OSLIBNAME);
  e_die();

  return Lua_RunFile(Lua_State, options.file);
}
