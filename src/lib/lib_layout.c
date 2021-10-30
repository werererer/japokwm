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
    {"get_focused", lib_layout_get_focused},
    {"load", lib_layout_load},
    {"load_in_set", lib_layout_load_in_set},
    {"load_next_in_set", lib_layout_load_next_in_set},
    {"load_prev_in_set", lib_layout_load_prev_in_set},
    {NULL, NULL},
};

static const struct luaL_Reg layout_m[] = {
    {"decrease_n_master", lib_decrease_n_master},
    {"increase_n_master", lib_increase_n_master},
    {"set", lib_layout_set_layout},
    {"set_linked_layouts", lib_layout_set_linked_layouts_ref},
    {"set_master_layout_data", lib_layout_set_master_layout_data},
    {"set_resize_data", lib_layout_set_resize_data},
    {"set_resize_function", lib_layout_set_resize_function},
    {NULL, NULL},
};

static const struct luaL_Reg layout_getter[] = {
    {"direction", lib_layout_get_direction},
    {"layout_data", lib_layout_get_layout_data},
    {"n_area", lib_layout_get_n_area},
    {"n_master", lib_layout_get_n_master},
    {"name", lib_layout_get_layout_name},
    {"o_layout_data", lib_layout_get_o_layout_data},
    {"resize_data", lib_layout_get_resize_data},
    {NULL, NULL},
};

static const struct luaL_Reg layout_setter[] = {
    {"default_layout", lib_layout_set_default_layout},
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
int lib_layout_get_focused(lua_State *L)
{
    struct workspace *ws = server_get_selected_workspace();
    struct layout *lt = workspace_get_layout(ws);

    create_lua_layout(L, lt);
    return 1;
}

int lib_layout_load(lua_State *L)
{
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    push_layout(ws, layout_name);

    arrange();
    return 0;
}

int lib_layout_load_in_set(lua_State *L)
{
    server.layout_set.lua_layout_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    // if nil return
    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, layout_set_key)) {
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);

    server.layout_set.key = layout_set_key;

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    layout_set_set_layout(ws);

    arrange();
    return 0;
}

int lib_layout_load_next_in_set(lua_State *L)
{
    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, layout_set_key)) {
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);
    server.layout_set.key = layout_set_key;

    // arg1
    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    lua_get_layout_set_element(L, layout_set_key);
    int n_layouts = luaL_len(L, -1);
    lua_pop(L, 2);

    server.layout_set.lua_layout_index++;
    if (server.layout_set.lua_layout_index > n_layouts) {
        server.layout_set.lua_layout_index = 1;
    }

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    layout_set_set_layout(ws);

    arrange();
    return 0;
}

int lib_layout_load_prev_in_set(lua_State *L)
{
    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    if (!lua_is_index_defined(L, layout_set_key)) {
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);
    server.layout_set.key = layout_set_key;

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    lua_get_layout_set_element(L, layout_set_key);
    int n_layouts = luaL_len(L, -1);
    lua_pop(L, 2);

    server.layout_set.lua_layout_index--;
    if (server.layout_set.lua_layout_index <= 0) {
        server.layout_set.lua_layout_index = n_layouts;
    }

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    layout_set_set_layout(ws);

    arrange();
    return 0;
}

// methods
int lib_decrease_n_master(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->n_master = MAX(lt->n_master-1, 1);
    arrange();
    return 1;
}

int lib_increase_n_master(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_master_layout_data_ref);
    int len = luaL_len(L, -1);
    lua_pop(L, 1);

    lt->n_master = MIN(lt->n_master+1, len);
    arrange();
    return 1;
}

int lib_layout_set_layout(lua_State *L)
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

int lib_layout_set_master_layout_data(lua_State *L)
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

int lib_layout_set_linked_layouts_ref(lua_State *L)
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

int lib_layout_set_resize_data(lua_State *L)
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

int lib_layout_set_resize_function(lua_State *L)
{
    int ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &ref);

    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->lua_resize_function_ref = ref;
    return 0;
}

// getter
int lib_layout_get_layout_name(lua_State *L)
{
    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);
    lua_pushstring(L, lt->name);
    return 1;
}

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
int lib_layout_set_default_layout(lua_State *L)
{
    const char *name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct layout *lt = check_layout(L, 1);
    lua_pop(L, 1);

    lt->name = name;
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
