#include "lib/lib_layout.h"

#include "layout.h"
#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "tag.h"
#include "translationLayer.h"
#include "utils/parseConfigUtils.h"
#include "tile/tileUtils.h"

#include <wlr/util/edges.h>

static const struct luaL_Reg layout_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg layout_static_getter[] =
{
    {"focused", lib_layout_get_focused},
    {NULL, NULL},
};

static const struct luaL_Reg layout_static_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg layout_static_methods[] =
{
    {"load", lib_layout_load},
    {"toggle", lib_layout_toggle},
    {NULL, NULL},
};

static const struct luaL_Reg layout_methods[] = {
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
            layout_static_methods,
            layout_methods,
            layout_setter,
            layout_getter,
            CONFIG_LAYOUT);

    create_static_accessor(L,
            "Layout",
            layout_static_methods,
            layout_static_setter,
            layout_static_getter);
}

struct layout *check_layout(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_LAYOUT);
    luaL_argcheck(L, ud != NULL, argn, "`layout' expected");
    return (struct layout *)*ud;
}

// static methods
int lib_layout_load(lua_State *L)
{
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    push_layout(tag, strdup(layout_name));

    arrange();
    return 0;
}

int lib_layout_toggle(lua_State *L)
{
    const char *desired_layout = luaL_checkstring(L, 1);
    lua_pop(L, 1);

    struct tag *tag = server_get_selected_tag();
    struct layout *lt = tag_get_layout(tag);
    bool is_layout = strcmp(lt->name, desired_layout) == 0;

    if (is_layout) {
        struct layout *prev_layout = tag_get_previous_layout(tag);
        push_layout(tag, strdup(prev_layout->name));
    } else {
        push_layout(tag, strdup(desired_layout));
    }

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
        const char *layout_name = get_config_array_str(L, "tags", i+1);
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
int lib_layout_get_focused(lua_State *L)
{
    struct tag *tag = server_get_selected_tag();
    struct layout *lt = tag_get_layout(tag);

    create_lua_layout(L, lt);
    return 1;
}

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
