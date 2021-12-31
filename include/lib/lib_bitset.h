#ifndef LIB_BITSET_H
#define LIB_BITSET_H

#include <lua.h>
#include <lauxlib.h>

#include "bitset/bitset.h"

struct tag_tags;

BitSet *check_bitset(lua_State *L, int narg);
void create_lua_bitset(lua_State *L, struct BitSet *bitset);
void create_lua_bitset_with_tag(lua_State *L, BitSet *bitset);
void create_lua_bitset_with_container(lua_State *L, BitSet *bitset);
void lua_load_bitset(lua_State *L);

# define call_bitset_func(L, action, self, ...) \
    do {\
        action(self, ##__VA_ARGS__);\
        if (luaL_testudata(L, 1, CONFIG_BITSET_WITH_TAG)) {\
            struct tag *tag = self->data;\
            tagset_set_tags(tag, self);\
            continue;\
        }\
        if (luaL_testudata(L, 1, CONFIG_BITSET_WITH_CONTAINER)) {\
            struct client *c = self->data;\
            client_setsticky(c, self);\
            continue;\
        }\
        \
    } while (0)

// meta functions
int lib_bitset_get(lua_State *L);
int lib_bitset_set(lua_State *L);
int lib_bitset_meta_bxor(lua_State *L);
int lib_bitset_meta_band(lua_State * L);
int lib_bitset_meta_bor(lua_State * L);
int lib_bitset_meta_bnot(lua_State * L);

int lib_bitset_gc(lua_State *L);
int lib_bitset_tostring(lua_State *L);

// static methods
int lib_bitset_new(lua_State *L);
// methods
int lib_bitset_and(lua_State *L);
int lib_bitset_not(lua_State *L);
int lib_bitset_or(lua_State *L);
int lib_bitset_xor(lua_State *L);
// getters
// setters


#endif /* LIB_BITSET_H */
