#ifndef NBFC_LUA_BINDINGS_H_
#define NBFC_LUA_BINDINGS_H_

#include "error.h"

#include <stdint.h>
#include <lua.h>

Error Lua_Open(void);
Error Lua_UseLibrary(const char*);
Error Lua_LoadCode(const char*, int*);
Error Lua_Call(int, uint64_t, uint64_t*);
int   Lua_ReturnError(lua_State*, const char*);
int   Lua_ReturnInteger(lua_State*, uint64_t);
int   Lua_ReturnString(lua_State*, const char*);
void  Lua_Close(void);

#endif
