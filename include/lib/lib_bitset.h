#ifndef LIB_BITSET_H
#define LIB_BITSET_H

#include <lua.h>
#include <lauxlib.h>

#include "bitset/bitset.h"

struct workspace_tags;

BitSet *check_bitset(lua_State *L, int narg);
void create_lua_bitset(struct BitSet *bitset);
void create_lua_bitset_with_workspace(BitSet *bitset);
void create_lua_bitset_with_container(BitSet *bitset);
void lua_load_bitset();

// meta functions
int lib_bitset_meta_bxor(lua_State *L);
int lib_bitset_meta_band(lua_State * L);
int lib_bitset_meta_bor(lua_State * L);
int lib_bitset_meta_bnot(lua_State * L);

int lib_bitset_gc(lua_State *L);

// functions
int lib_bitset_new(lua_State *L);
// methods
int lib_bitset_and(lua_State *L);
int lib_bitset_not(lua_State *L);
int lib_bitset_or(lua_State *L);
int lib_bitset_xor(lua_State *L);
// getters
// setters


#endif /* LIB_BITSET_H */
