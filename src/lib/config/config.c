#include "lib/config/config.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"
#include "monitor.h"
#include "workspace.h"

int lib_reload(lua_State *L)
{
    struct workspace *ws = get_workspace(selected_monitor->ws_selector.ws_id);
    server.default_layout->options = get_default_options();

    load_config(L);

    update_workspaces(&server.workspaces, &server.default_layout->options.tag_names);
    reset_loaded_layouts(&server.workspaces);

    ipc_event_workspace();
    load_default_layout(L, ws);

    arrange();
    return 0;
}

int lib_set_arrange_by_focus(lua_State *L)
{
    struct layout *lt = server.default_layout;

    // 1. argument
    lt->options.arrange_by_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_inner_gaps(lua_State *L)
{
    struct layout *lt = server.default_layout;

    lt->options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_outer_gaps(lua_State *L)
{
    server.default_layout->options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_tile_borderpx(lua_State *L)
{
    server.default_layout->options.tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_float_borderpx(lua_State *L)
{
    server.default_layout->options.float_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_focus_color(lua_State *L)
{
    lua_tocolor(server.default_layout->options.focus_color);
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
    i = MAX(MIN(i, 3), 0);

    server.default_layout->options.modkey = i;
    lua_pop(L, 1);
    return 0;
}

int lib_set_border_color(lua_State *L)
{
    lua_tocolor(server.default_layout->options.border_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    lua_tocolor(server.default_layout->options.root_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_sloppy_focus(lua_State *L)
{
    server.default_layout->options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_rate(lua_State *L)
{
    server.default_layout->options.repeat_rate = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_delay(lua_State *L)
{
    server.default_layout->options.repeat_delay = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_default_layout(lua_State *L)
{
    const char *name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    server.default_layout->name = name;
    return 0;
}

// TODO refactor this function hard to read
int lib_create_workspaces(lua_State *L)
{
    struct wlr_list *tag_names = &server.default_layout->options.tag_names;
    wlr_list_clear(tag_names, NULL);

    size_t len = lua_rawlen(L, -1);
    for (int i = 0; i < len; i++) {
        char *ws_name = get_config_array_str(L, "workspaces", i+1);
        wlr_list_push(tag_names, ws_name);
    }
    lua_pop(L, 1);

    update_workspaces(&server.workspaces, tag_names);

    ipc_event_workspace();

    return 0;
}

int lib_set_rules(lua_State *L)
{
    if (server.default_layout->options.rules)
        free(server.default_layout->options.rules);

    size_t len = lua_rawlen(L, -1);
    server.default_layout->options.rule_count = len;
    server.default_layout->options.rules = calloc(len, sizeof(struct rule));
    struct rule *rules = server.default_layout->options.rules;

    for (int i = 0; i < server.default_layout->options.rule_count; i++) {
        struct rule r = get_config_array_rule(L, "rules", i+1);
        rules[i] = r;
    }

    lua_pop(L, 1);
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

int lib_set_monrules(lua_State *L)
{
    if (server.default_layout->options.monrules)
        free(server.default_layout->options.monrules);

    size_t len = lua_rawlen(L, -1);
    server.default_layout->options.monrule_count = len;
    server.default_layout->options.monrules = calloc(len, sizeof(struct monrule));
    struct monrule *rules = server.default_layout->options.monrules;

    for (int i = 0; i < len; i++) {
        struct monrule r = get_config_array_monrule(L, "rules", i+1);
        rules[i] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_keybinds(lua_State *L)
{
    lua_copy_table_safe(L, &server.default_layout->options.keybinds_ref);
    return 0;
}

int lib_set_layout_constraints(lua_State *L)
{
    server.default_layout->options.layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_constraints(lua_State *L)
{
    server.default_layout->options.master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_resize_direction(lua_State *L)
{
    server.default_layout->options.resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_layout_data(lua_State *L)
{
    if (lua_islayout_data(L, "master_layout_data"))
        lua_copy_table_safe(L, &server.default_layout->lua_master_layout_data_ref);
    else
        lua_pop(L, 1);
    return 0;
}

int lib_set_hidden_edges(lua_State *L)
{
    server.default_layout->options.hidden_edges = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_smart_hidden_edges(lua_State *L)
{
    server.default_layout->options.smart_hidden_edges = lua_toboolean(L, -1);
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
