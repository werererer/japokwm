#ifndef LIB_GMP_H
#define LIB_GMP_H

#include <lua.h>
#include <lauxlib.h>

#include <mpfr.h>

void lua_load_gmp(lua_State *L);
int lib_gmp_new(lua_State *L);

// meta
int lib_gmp_gc(lua_State *L);
int lib_gmp_tostring(lua_State *L);
int lib_gmp_add(lua_State *L);
int lib_gmp_sub(lua_State *L);
int lib_gmp_mul(lua_State *L);
int lib_gmp_div(lua_State *L);
int lib_gmp_pow(lua_State *L);

mpfr_ptr check_gmp(lua_State *L, int argn);

#endif
