#ifndef LIB_GMP_H
#define LIB_GMP_H

#include <lua.h>
#include <lauxlib.h>

#include <mpfr.h>

void lua_load_gmp(lua_State *L);

// meta
int lib_gmp_gc(lua_State *L);
int lib_gmp_tostring(lua_State *L);
int lib_gmp_add(lua_State *L);
int lib_gmp_sub(lua_State *L);
int lib_gmp_mul(lua_State *L);
int lib_gmp_div(lua_State *L);
int lib_gmp_pow(lua_State *L);
int lib_gmp_lt(lua_State *L);
int lib_gmp_le(lua_State *L);

mpfr_ptr check_gmp(lua_State *L, int argn);

// static methods
int lib_gmp_new(lua_State *L);
int lib_gmp_max(lua_State *L);
int lib_gmp_min(lua_State *L);
// getter
int lib_gmp_get_value(lua_State *L);


#endif
