#include "lib/lib_bitset.h"

#include "server.h"
#include "translationLayer.h"
#include "bitset/bitset.h"
#include "workspace.h"
#include "tagset.h"
#include "lib/lib_list.h"

static const struct luaL_Reg bitset_gc_meta[] =
{
    {"__gc", lib_bitset_gc},
    {NULL, NULL},
};

static const struct luaL_Reg bitset_meta[] =
{
    {"__index", lib_bitset_get},
    {"__newindex", lib_bitset_set},
    {"__bxor", lib_bitset_meta_bxor},
    {"__band", lib_bitset_meta_band},
    {"__bor", lib_bitset_meta_bor},
    {"__bnot", lib_bitset_meta_bnot},
    {NULL, NULL},
};

static const struct luaL_Reg bitset_f[] =
{
    {"new", lib_bitset_new},
    {NULL, NULL},
};

static const struct luaL_Reg bitset_m[] =
{
    {"_xor", lib_bitset_xor},
    {"_and", lib_bitset_and},
    {"_or", lib_bitset_or},
    {"_not", lib_bitset_not},
    {NULL, NULL},
};

static const struct luaL_Reg bitset_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg bitset_getter[] =
{
    {NULL, NULL},
};

void lua_load_bitset(lua_State *L)
{
    create_class(L,
            bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET);

    create_class(L,
            bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET_WITH_WORKSPACE);

    create_class(L,
            bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET_WITH_CONTAINER);

    create_class(L,
            bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET_GC);
    luaL_getmetatable(L, CONFIG_BITSET_GC);
    luaL_setfuncs(L, bitset_gc_meta, 0);
    lua_pop(L, 1);

    luaL_newlib(L, bitset_f);
    lua_setglobal(L, "Bitset");
}

static void create_lua_bitset_gc(lua_State *L, BitSet *bitset)
{
    BitSet **user_bitset = lua_newuserdata(L, sizeof(BitSet *));
    *user_bitset = bitset;

    luaL_setmetatable(L, CONFIG_BITSET_GC);
}

BitSet *check_bitset(lua_State *L, int narg)
{
    void **ud = NULL;
    if (luaL_testudata(L, narg, CONFIG_BITSET)) {
        ud = luaL_checkudata(L, narg, CONFIG_BITSET);
    } else if (luaL_testudata(L, narg, CONFIG_BITSET_GC)) {
        ud = luaL_checkudata(L, narg, CONFIG_BITSET_GC);
    } else if (luaL_testudata(L, narg, CONFIG_BITSET_WITH_CONTAINER)) {
        ud = luaL_checkudata(L, narg, CONFIG_BITSET_WITH_CONTAINER);
    } else if (luaL_testudata(L, narg, CONFIG_BITSET_WITH_WORKSPACE)) {
        ud = luaL_checkudata(L, narg, CONFIG_BITSET_WITH_WORKSPACE);
    } else if (lua_isinteger(L, narg)) {
        int tags_dec = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        BitSet *tmp_bitset = bitset_from_value(tags_dec);
        BitSet *bitset = bitset_create();
        for (int i = 0; i < bitset->size; i++) {
            int last_bit_id = tmp_bitset->size - 1;
            bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
        }
        bitset_destroy(tmp_bitset);

        // hand over garbage collection to lua
        create_lua_bitset_gc(L, bitset);
        ud = luaL_checkudata(L, narg, CONFIG_BITSET_GC);
    } else {
        lua_pushstring(L, "`bitset' expected");
        lua_error(L);
    }
    luaL_argcheck(L, ud != NULL, 1, "`bitset' expected");
    return *(BitSet **)ud;
}

void create_lua_bitset_with_workspace(lua_State *L, BitSet *bitset)
{
    BitSet **user_bitset = lua_newuserdata(L, sizeof(BitSet *));
    *user_bitset = bitset;

    luaL_setmetatable(L, CONFIG_BITSET_WITH_WORKSPACE);
}

void create_lua_bitset_with_container(lua_State *L, BitSet *bitset)
{
    BitSet **user_bitset = lua_newuserdata(L, sizeof(BitSet *));
    *user_bitset = bitset;

    luaL_setmetatable(L, CONFIG_BITSET_WITH_CONTAINER);
}

void create_lua_bitset(lua_State *L, struct BitSet *bitset)
{
    struct BitSet **user_tags = lua_newuserdata(L, sizeof(BitSet *));
    *user_tags = bitset;

    luaL_setmetatable(L, CONFIG_BITSET);
}

// meta
int lib_bitset_get(lua_State *L)
{
    // [table, key]
    const char *key = luaL_checkstring(L, -1); // convert lua to c index
    BitSet *bitset = check_bitset(L, 1);
    debug_print("key: %s\n", key);

    bool is_number = lua_isnumber(L, -1);
    if (!is_number) {
        get_lua_value(L);
        return 1;
    }

    int i = lua_tonumber(L, -1)-1;
    if (i < 0) {
        lua_pushnil(L);
        return 1;
    }

    bool b = bitset_test(bitset, i);
    lua_pushboolean(L, b);
    return 1;
}

int lib_bitset_set(lua_State *L)
{
    // [table, key, value]
    bool value = lua_toboolean(L, 3);
    const char *key = luaL_checkstring(L, 2); // convert lua to c index
    BitSet *bitset = check_bitset(L, 1);
    debug_print("key: %s\n", key);

    bool is_number = lua_isnumber(L, 2);
    if (!is_number) {
        set_lua_value(L);
        return 0;
    }

    int i = lua_idx_to_c_idx(lua_tonumber(L, 2));
    if (i < 0) {
        lua_pushnil(L);
        return 1;
    }

    call_bitset_func(L, bitset_assign, bitset, i, value);
    return 0;
}

int lib_bitset_meta_bxor(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *self = check_bitset(L, 1);
    lua_pop(L, 1);

    BitSet *target_bitset = bitset_create();
    bitset_assign_bitset(&target_bitset, self);
    bitset_xor(target_bitset, bitset);

    create_lua_bitset_gc(L, target_bitset);
    return 1;
}

int lib_bitset_meta_band(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *oth_bitset = check_bitset(L, 1);
    lua_pop(L, 1);

    BitSet *target_bitset = bitset_create();
    bitset_assign_bitset(&target_bitset, oth_bitset);
    bitset_and(target_bitset, bitset);

    create_lua_bitset_gc(L, target_bitset);
    return 1;
}

int lib_bitset_meta_bor(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *oth_bitset = check_bitset(L, 1);
    lua_pop(L, 1);

    BitSet *target_bitset = bitset_create();
    bitset_assign_bitset(&target_bitset, oth_bitset);
    bitset_or(target_bitset, bitset);

    create_lua_bitset_gc(L, target_bitset);
    return 1;
}

int lib_bitset_meta_bnot(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 1);
    lua_pop(L, 1);

    BitSet *target_bitset = bitset_create();
    bitset_assign_bitset(&target_bitset, bitset);
    bitset_flip(target_bitset);

    create_lua_bitset_gc(L, target_bitset);
    return 1;
}

int lib_bitset_gc(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 1);
    lua_pop(L, 1);
    bitset_destroy(bitset);
    return 0;
}

// functions
int lib_bitset_new(lua_State *L)
{
    BitSet *bitset = bitset_create();
    // hand over rights for this pointer (deletion etc.) to lua
    create_lua_bitset_gc(L, bitset);
    return 1;
}
// methods

int lib_bitset_xor(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *self = check_bitset(L, 1);
    call_bitset_func(L, bitset_xor, self, bitset);
    lua_pop(L, 1);
    bitset_destroy(self);

    return 0;
}

int lib_bitset_and(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *self = check_bitset(L, 1);
    call_bitset_func(L, bitset_and, self, bitset);
    lua_pop(L, 1);

    return 0;
}

int lib_bitset_or(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *self = check_bitset(L, 1);
    call_bitset_func(L, bitset_or, self, bitset);
    lua_pop(L, 1);

    return 0;
}

int lib_bitset_not(lua_State *L)
{
    BitSet *self = check_bitset(L, 1);
    call_bitset_func(L, bitset_flip, self);
    lua_pop(L, 1);
    return 0;
}
// getters
// setters
