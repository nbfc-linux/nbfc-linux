#ifndef NBFC_LUA_BINDINGS_H_
#define NBFC_LUA_BINDINGS_H_

#include "error.h"

#include <stdint.h>

Error Lua_UseLibrary(const char*);
Error Lua_LoadCode(const char*, int*);
Error Lua_Call(int, uint64_t, uint64_t*);
void  Lua_Close(void);

#endif
