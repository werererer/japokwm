#include "lib/lib_layout.h"

#include "layout.h"
#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "workspace.h"
#include "translationLayer.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"

#include <wlr/util/edges.h>

static const struct luaL_Reg layout_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg layout_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg layout_m[] = {
    {"set", lib_set_layout},
    {"set_linked_layouts", lib_set_linked_layouts_ref},
    {"set_master_layout_data", lib_set_master_layout_data},
    {"set_resize_data", lib_set_resize_data},
    {"set_resize_function", lib_set_resize_function},
    {NULL, NULL},
};

static const struct luaL_Reg layout_getter[] = {
    {"direction", lib_layout_get_direction},
    {"layout_data", lib_layout_get_layout_data},
    {"n_area", lib_layout_get_n_area},
    {"n_master", lib_layout_get_n_master},
    {"o_layout_data", lib_layout_get_o_layout_data},
    {"resize_data", lib_layout_get_resize_data},
    {NULL, NULL},
};

static const struct luaL_Reg layout_setter[] = {
    {"default_layout", lib_set_default_layout},
    {"n_area", lib_layout_set_n_area},
    {"n_master", lib_layout_set_n_area},
    // {"direction", lib_layout_get_direction},
    // {"layout_data", lib_layout_get_layout_data},
    // {"o_layout_data", lib_layout_get_o_layout_data},
    // {"resize_data", lib_layout_get_resize_data},
    {NULL, NULL},
};

void create_lua_layout(lua_State *L, struct layout *layout)
{
    if (!layout)
        return;
    struct layout **user_con = lua_newuserdata(L, sizeof(struct layout*));
    *user_con = layout;

    luaL_setmetatable(L, CONFIG_LAYOUT);
}

void lua_init_layout(struct layout *layout)
{
    create_lua_layout(L, layout);
    lua_setglobal(L, "layout");
}

void lua_load_layout(lua_State *L)
{
    create_class(L,
            layout_meta,
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

// functions
int lib_get_active_layout(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);

    struct layout *lt = workspace_get_layout(ws);
    lua_pushstring(L, lt->symbol);
    return 1;
}


// methods
int lib_set_layout(lua_State *L)
{
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
    // stack: [layout, master_layout_data]
    lua_insert(L, -2);
    // stack: [master_layout_data, layout]
    struct layout *lt = check_layout(L, 2);
    lua_pop(L, 1);

    if (lua_is_layout_data(L, "master_layout_data")) {
        lua_copy_table_safe(L, &lt->lua_master_layout_data_ref);
    } else {
        lua_pop(L, 1);
    }
    return 0;
}

int lib_set_linked_layouts_ref(lua_State *L)
{
    // stack: [layout, master_layout_data]
    lua_insert(L, -2);
    // stack: [master_layout_data, layout]
    struct layout *lt = check_layout(L, 2);
    lua_pop(L, 1);

    size_t len = lua_rawlen(L, -1);
    for (int i = 0; i < len; i++) {
        const char *layout_name = get_config_array_str(L, "workspaces", i+1);
        g_ptr_array_add(lt->linked_layouts, strdup(layout_name));
    }
    lua_pop(L, 1);
    return 0;
}

int lib_set_resize_data(lua_State *L)
{
    // stack: [layout, master_layout_data]
    lua_insert(L, -2);
    // stack: [master_layout_data, layout]
    struct layout *lt = check_layout(L, 2);
    lua_pop(L, 1);

    if (lua_istable(L, -1))
        lua_copy_table_safe(L, &lt->lua_resize_data_ref);
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

// getter
int lib_layout_get_direction(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    enum wlr_edges resize_dir = lt->options->resize_dir;
    lua_pushinteger(L, resize_dir);
    return 1;
}

int lib_layout_get_layout_data(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_copy_data_ref);
    return 1;
}

int lib_layout_get_o_layout_data(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_layout_original_copy_data_ref);
    return 1;
}

int lib_layout_get_resize_data(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_resize_data_ref);
    return 1;
}

// setter
int lib_set_default_layout(lua_State *L)
{
    const char *symbol = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->symbol = symbol;
    return 0;
}

int lib_layout_set_n_area(lua_State *L)
{
    int current_max_area = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->current_max_area = current_max_area;
    arrange();
    return lt->current_max_area;
}

int lib_layout_set_nmaster(lua_State *L)
{
    int nmaster = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->n_master = nmaster;
    return 0;
}

int lib_layout_get_n_area(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    int current_max_area = lt->current_max_area;
    lua_pushinteger(L, current_max_area);
    return 1;
}

int lib_layout_get_n_master(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    int n_master = lt->n_master;
    lua_pushinteger(L, n_master);
    return 1;
}

int lib_layout_get_n_tiled(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    int n_tiled = lt->n_tiled;
    lua_pushinteger(L, n_tiled);
    return 1;
}
// getter
