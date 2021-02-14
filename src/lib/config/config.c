#include "lib/config/config.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "ipc-server.h"

int lib_reload(lua_State *L)
{
    init_config(L);

    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        struct workspace *ws = get_workspace(m->ws_ids[0]);
        load_layout(L, ws, ws->layout->name, ws->layout->symbol);
    }

    arrange();
    return 0;
}

int lib_set_gaps(lua_State *L)
{
    server.default_layout.options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    server.default_layout.options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    configure_gaps(&server.default_layout.options.inner_gap, &server.default_layout.options.outer_gap);
    return 0;
}

int lib_set_tile_borderpx(lua_State *L)
{
    server.default_layout.options.tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_float_borderpx(lua_State *L)
{
    server.default_layout.options.float_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_focus_color(lua_State *L)
{
    lua_tocolor(server.default_layout.options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_mod(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);

    // there are only 4 mods ranging from 1-4
    i = MAX(MIN(i, 3), 0);

    server.default_layout.options.modkey = i;
    lua_pop(L, 1);
    return 0;
}

int lib_set_border_color(lua_State *L)
{
    lua_tocolor(server.default_layout.options.border_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    lua_tocolor(server.default_layout.options.root_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_sloppy_focus(lua_State *L)
{
    server.default_layout.options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_rate(lua_State *L)
{
    server.default_layout.options.repeat_rate = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_delay(lua_State *L)
{
    server.default_layout.options.repeat_delay = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_default_layout(lua_State *L)
{
    lua_rawgeti(L, -1, 1);
    const char *symbol = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, -1, 2);
    const char *name = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    lua_pop(L, 1);

    server.default_layout.name = name;
    server.default_layout.symbol = symbol;
    return 0;
}

int lib_set_workspaces(lua_State *L)
{
    size_t len = lua_rawlen(L, -1);
    struct wlr_list *tag_names = &server.default_layout.options.tag_names;

    wlr_list_clear(tag_names);
    for (int i = 0; i < len; i++)
        wlr_list_push(&server.default_layout.options.tag_names,
                get_config_array_str(L, "workspaces", i+1));
    lua_pop(L, 1);

    destroy_workspaces();
    create_workspaces(*tag_names, server.default_layout);

    ipc_event_workspace();

    return 0;
}

int lib_set_rules(lua_State *L)
{
    if (server.default_layout.options.rules)
        free(server.default_layout.options.rules);

    size_t len = lua_rawlen(L, -1);
    server.default_layout.options.rule_count = len;
    server.default_layout.options.rules = calloc(len, sizeof(struct rule));
    struct rule *rules = server.default_layout.options.rules;

    for (int i = 0; i < server.default_layout.options.rule_count; i++) {
        struct rule r = get_config_array_rule(L, "rules", i+1);
        rules[i] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_layouts(lua_State *L)
{
    const int layout_set_ref = lua_copy_table(L);
    const char *layout_set_key = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    lua_rawgeti(L, LUA_REGISTRYINDEX, server.layout_set.layout_sets_ref);
    lua_set_layout_set_element(L, layout_set_key, layout_set_ref);
    lua_pop(L, 1);
    return 0;
}

int lib_set_monrules(lua_State *L)
{
    if (server.default_layout.options.monrules)
        free(server.default_layout.options.monrules);

    size_t len = lua_rawlen(L, -1);
    server.default_layout.options.monrule_count = len;
    server.default_layout.options.monrules = calloc(len, sizeof(struct monrule));
    struct monrule *monrules = server.default_layout.options.monrules;

    for (int i = 0; i < server.default_layout.options.monrule_count; i++) {
        struct monrule r = get_config_array_monrule(L, "monrules", i+1);
        monrules[i] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_keybinds(lua_State *L)
{
    if (server.default_layout.options.keybinds_ref > 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, server.default_layout.options.keybinds_ref);
    }
    server.default_layout.options.keybinds_ref = lua_copy_table(L);
    return 0;
}

int lib_set_buttons(lua_State *L)
{
    server.default_layout.options.buttonbindings_ref = lua_copy_table(L);
    return 0;
}

int lib_set_layout_constraints(lua_State *L)
{
    server.default_layout.options.layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_constraints(lua_State *L)
{
    server.default_layout.options.master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_update_function(lua_State *L)
{
    server.default_layout.options.update_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int lib_set_resize_direction(lua_State *L)
{
    server.default_layout.options.resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_layout_data(lua_State *L)
{
    if (lua_islayout_data(L, "master_layout_data"))
        server.default_layout.options.master_layout_data_ref = lua_copy_table(L);
    else
        lua_pop(L, 1);
    return 0;
}

int lib_set_resize_data(lua_State *L)
{
    if (lua_istable(L, -1))
        server.default_layout.options.resize_data_ref = lua_copy_table(L);
    else
        lua_pop(L, 1);
    return 0;
}
