#include "lib/config/lib_config.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"
#include "monitor.h"
#include "workspace.h"
#include "keybinding.h"
#include "rules/rule.h"
#include "rules/mon_rule.h"

int lib_reload(lua_State *L)
{
    close_error_file();
    init_error_file();

    options_reset(server.default_layout->options);

    debug_print("remove loaded layouts start\n");
    remove_loaded_layouts(server.workspaces);
    debug_print("remove loaded layouts end\n");
    load_config(L);
    for (int i = 0; i < server.workspaces->len; i++) {
        struct workspace *ws = g_ptr_array_index(server.workspaces, i);
        load_default_layout(ws);
    }

    ipc_event_workspace();

    arrange();
    return 0;
}

int lib_set_arrange_by_focus(lua_State *L)
{
    struct layout *lt = server.default_layout;

    // 1. argument
    lt->options->arrange_by_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_automatic_workspace_naming(lua_State *L)
{
    struct layout *lt = server.default_layout;

    // 1. argument
    lt->options->automatic_workspace_naming = lua_toboolean(L, -1);
    lua_pop(L, 1);
    ipc_event_workspace();
    return 0;
}

int lib_set_inner_gaps(lua_State *L)
{
    struct layout *lt = server.default_layout;

    lt->options->inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_outer_gaps(lua_State *L)
{
    server.default_layout->options->outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_tile_borderpx(lua_State *L)
{
    server.default_layout->options->tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_float_borderpx(lua_State *L)
{
    server.default_layout->options->float_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_focus_color(lua_State *L)
{
    lua_tocolor(server.default_layout->options->focus_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_hide_edge_borders(lua_State *L)
{
    // TODO lua implementation
    return 0;
}

int lib_set_mod(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);

    // there are only 4 mods ranging from 1-4
    i = MAX(MIN(i-1, 3), 0);

    server.default_layout->options->modkey = i;
    lua_pop(L, 1);
    return 0;
}

int lib_set_border_color(lua_State *L)
{
    lua_tocolor(server.default_layout->options->border_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    lua_tocolor(server.default_layout->options->root_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_sloppy_focus(lua_State *L)
{
    server.default_layout->options->sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_rate(lua_State *L)
{
    server.default_layout->options->repeat_rate = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_delay(lua_State *L)
{
    server.default_layout->options->repeat_delay = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_default_layout(lua_State *L)
{
    const char *symbol = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    server.default_layout->symbol = symbol;
    return 0;
}

int lib_set_entry_position_function(lua_State *L)
{
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);
    server.default_layout->options->new_position_func_ref = lua_func_ref;
    return 0;
}

int lib_set_entry_focus_position_function(lua_State *L)
{
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);
    server.default_layout->options->new_focus_position_func_ref = lua_func_ref;
    return 0;
}

// TODO refactor this function hard to read
int lib_create_workspaces(lua_State *L)
{
    GPtrArray *tag_names = server.default_layout->options->tag_names;
    list_clear(tag_names, NULL);

    size_t len = lua_rawlen(L, -1);
    for (int i = 0; i < len; i++) {
        const char *ws_name = get_config_array_str(L, "workspaces", i+1);
        g_ptr_array_add(tag_names, strdup(ws_name));
    }
    lua_pop(L, 1);

    return 0;
}

int lib_add_rule(lua_State *L)
{
    GPtrArray *rules = server.default_layout->options->rules;
    struct rule *rule = get_config_rule(L);
    lua_pop(L, 1);

    g_ptr_array_add(rules, rule);
    return 0;
}

int lib_create_layout_set(lua_State *L)
{
    int *layout_set_ref = &server.layout_set.layout_sets_ref;
    lua_copy_table_safe(L, layout_set_ref);
    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, *layout_set_ref);
    lua_set_layout_set_element(L, layout_set_key, *layout_set_ref);
    lua_pop(L, 1);
    return 0;
}

int lib_add_mon_rule(lua_State *L)
{
    struct mon_rule *mon_rule = get_config_mon_rule(L);
    lua_pop(L, 1);

    GPtrArray *mon_rules = server.default_layout->options->mon_rules;
    g_ptr_array_add(mon_rules, mon_rule);
    return 0;
}

int lib_bind_key(lua_State *L)
{
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);
    const char *binding = luaL_checkstring(L, -1);

    struct keybinding *keybinding = create_keybinding(binding, lua_func_ref);
    lua_pop(L, 1);
    g_ptr_array_add(server.default_layout->options->keybindings, keybinding);
    return 0;
}

int lib_set_layout_constraints(lua_State *L)
{
    server.default_layout->options->layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_constraints(lua_State *L)
{
    server.default_layout->options->master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_resize_direction(lua_State *L)
{
    server.default_layout->options->resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_resize_function(lua_State *L)
{
    lua_ref_safe(L, LUA_REGISTRYINDEX, &server.default_layout->lua_resize_function_ref);
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

int lib_set_hidden_edges(lua_State *L)
{
    server.default_layout->options->hidden_edges = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_smart_hidden_edges(lua_State *L)
{
    server.default_layout->options->smart_hidden_edges = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_resize_data(lua_State *L)
{
    if (lua_istable(L, -1))
        lua_copy_table_safe(L, &server.default_layout->lua_resize_data_ref);
    else
        lua_pop(L, 1);
    return 0;
}
