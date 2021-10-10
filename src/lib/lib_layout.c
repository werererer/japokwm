#include "lib/lib_layout.h"

#include "layout.h"
#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "workspace.h"
#include "translationLayer.h"

static const struct luaL_Reg layout_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg layout_m[] = {
    {"set", lib_set_layout},
    {"set_default_layout", lib_set_default_layout},
    {"set_resize_function", lib_set_resize_function},
    {NULL, NULL},
};

static const struct luaL_Reg layout_getter[] = {
    {NULL, NULL},
};

static const struct luaL_Reg layout_setter[] = {
    {NULL, NULL},
};

void create_lua_layout(struct layout *layout)
{
    if (!layout)
        return;
    struct layout **user_con = lua_newuserdata(L, sizeof(struct layout*));
    *user_con = layout;

    luaL_setmetatable(L, CONFIG_LAYOUT);
}

void lua_init_layout(struct layout *layout)
{
    create_lua_layout(layout);
    lua_setglobal(L, "layout");
}

void lua_load_layout()
{
    create_class(
            layout_f,
            layout_m,
            layout_setter,
            layout_getter,
            CONFIG_LAYOUT);

    luaL_newlib(L, layout_f);
    lua_setglobal(L, "Layout");
}

struct layout *check_layout(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_LAYOUT);
    luaL_argcheck(L, ud != NULL, argn, "`layout' expected");
    return (struct layout *)*ud;
}

int lib_set_default_layout(lua_State *L)
{
    const char *symbol = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    server.default_layout->symbol = symbol;
    return 0;
}

// functions
// methods
int lib_set_layout(lua_State *L)
{
    debug_print("lib set layout\n");
    int ref = 0;
    // 1. argument -- layout_set
    if (lua_is_layout_data(L, "layout_data")) {
        lua_ref_safe(L, LUA_REGISTRYINDEX, &ref);
    } else {
        lua_pop(L, 1);
    }
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    if (ref > 0) {
        lt->lua_layout_copy_data_ref = ref;
    }
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    lua_copy_table_safe(L, &lt->lua_layout_original_copy_data_ref);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_layout_data(lua_State *L)
{
    if (lua_is_layout_data(L, "master_layout_data"))
        lua_copy_table_safe(L, &server.default_layout->lua_master_layout_data_ref);
    else
        lua_pop(L, 1);
    return 0;
}

int lib_set_resize_function(lua_State *L)
{
    int ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &ref);

    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->lua_resize_function_ref = ref;
    return 0;
}

// setter
// getter
