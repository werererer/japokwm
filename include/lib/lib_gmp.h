#ifndef LIB_GMP_H
#define LIB_GMP_H

#include <lua.h>
#include <lauxlib.h>

#include <mpfr.h>

void lua_load_gmp(lua_State *L);
int lib_gmp_new(lua_State *L);

struct color check_gmp(lua_State *L, int narg);

#endif
