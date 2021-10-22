#include "lib/lib_bitset.h"

#include "server.h"
#include "translationLayer.h"
#include "bitset/bitset.h"
#include "workspace.h"
#include "tagset.h"

static const struct luaL_Reg bitset_gc_meta[] =
{
    {"__gc", lib_bitset_gc},
    {NULL, NULL},
};

static const struct luaL_Reg bitset_meta[] =
{
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

void lua_load_bitset()
{
    create_class(bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET);

    create_class(bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET_WITH_WORKSPACE);

    create_class(bitset_meta,
            bitset_f,
            bitset_m,
            bitset_setter,
            bitset_getter,
            CONFIG_BITSET_WITH_CONTAINER);

    create_class(bitset_meta,
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

static void create_lua_bitset_gc(BitSet *bitset)
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
        create_lua_bitset_gc(bitset);
        ud = luaL_checkudata(L, narg, CONFIG_BITSET_GC);
    } else {
        lua_pushstring(L, "`bitset' expected");
        lua_error(L);
    }
    luaL_argcheck(L, ud != NULL, 1, "`bitset' expected");
    return *(BitSet **)ud;
}

void create_lua_bitset_with_workspace(BitSet *bitset)
{
    BitSet **user_bitset = lua_newuserdata(L, sizeof(BitSet *));
    *user_bitset = bitset;

    luaL_setmetatable(L, CONFIG_BITSET_WITH_WORKSPACE);
}

void create_lua_bitset_with_container(BitSet *bitset)
{
    BitSet **user_bitset = lua_newuserdata(L, sizeof(BitSet *));
    *user_bitset = bitset;

    luaL_setmetatable(L, CONFIG_BITSET_WITH_CONTAINER);
}

void create_lua_bitset(struct BitSet *bitset)
{
    struct BitSet **user_tags = lua_newuserdata(L, sizeof(BitSet *));
    *user_tags = bitset;

    luaL_setmetatable(L, CONFIG_BITSET);
}

// meta
int lib_bitset_meta_bxor(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *self = check_bitset(L, 1);
    lua_pop(L, 1);

    BitSet *target_bitset = bitset_create();
    bitset_assign_bitset(&target_bitset, self);
    bitset_xor(target_bitset, bitset);

    create_lua_bitset_gc(target_bitset);
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

    create_lua_bitset_gc(target_bitset);
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

    create_lua_bitset_gc(target_bitset);
    return 1;
}

int lib_bitset_meta_bnot(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 1);
    lua_pop(L, 1);

    BitSet *target_bitset = bitset_create();
    bitset_assign_bitset(&target_bitset, bitset);
    bitset_flip(target_bitset);

    create_lua_bitset_gc(target_bitset);
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
    create_lua_bitset_gc(bitset);
    return 1;
}
// methods
# define call_bitset_func(L, action, ...) \
    do {\
        action(__VA_ARGS__);\
        if (luaL_testudata(L, 1, CONFIG_BITSET_WITH_WORKSPACE)) {\
            struct workspace *ws = self->data;\
            tagset_set_tags(ws, self);\
            return 0;\
        }\
        if (luaL_testudata(L, 1, CONFIG_BITSET_WITH_CONTAINER)) {\
            struct workspace *ws = self->data;\
            tagset_set_tags(ws, self);\
            return 0;\
        }\
        \
    } while (0)

int lib_bitset_xor(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    BitSet *self = check_bitset(L, 1);
    call_bitset_func(L, bitset_xor, self, bitset);
    lua_pop(L, 1);

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
