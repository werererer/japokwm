#include "lib/config/config.h"
#include "utils/gapUtils.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "server.h"

int lib_set_gaps(lua_State *L)
{
    server.options.outer_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    server.options.inner_gap = luaL_checkinteger(L ,-1);
    lua_pop(L, 1);
    configure_gaps(&server.options.inner_gap, &server.options.outer_gap);
    return 0;
}

int lib_set_tile_borderpx(lua_State *L)
{
    server.options.tile_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_float_borderpx(lua_State *L)
{
    server.options.float_border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_focus_color(lua_State *L)
{
    lua_tocolor(server.options.focus_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_mod(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);

    // there are only 4 mods ranging from 1-4
    i = MAX(MIN(i, 3), 0);

    server.options.modkey = i;
    lua_pop(L, 1);
    return 0;
}

int lib_set_border_color(lua_State *L)
{
    lua_tocolor(server.options.border_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    lua_tocolor(server.options.root_color);
    lua_pop(L, 1);
    return 0;
}

int lib_set_sloppy_focus(lua_State *L)
{
    server.options.sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_rate(lua_State *L)
{
    server.options.repeat_rate = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_repeat_delay(lua_State *L)
{
    server.options.repeat_delay = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_default_layout(lua_State *L)
{
    struct layout layout = {
        .symbol = "s",
        .name = "master",
        .n = 1,
        .nmaster = 1,
    };
    server.default_layout = layout;
    lua_pop(L, 1);
    return 0;
}

int lib_set_workspaces(lua_State *L)
{
    size_t len = lua_rawlen(L, -1);

    wlr_list_clear(&server.options.tag_names);
    for (int i = 0; i < len; i++)
        wlr_list_push(&server.options.tag_names, get_config_array_str(L, "workspaces", i+1));
    lua_pop(L, 1);
    return 0;
}

int lib_set_rules(lua_State *L)
{
    if (server.options.rules)
        free(server.options.rules);

    size_t len = lua_rawlen(L, -1);
    server.options.rule_count = len;
    server.options.rules = calloc(len, sizeof(struct rule));
    struct rule *rules = server.options.rules;

    for (int i = 0; i < server.options.rule_count; i++) {
        struct rule r = get_config_array_rule(L, "rules", i+1);
        rules[i] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_layouts(lua_State *L)
{
    server.options.layouts_ref = lua_copy_table(L);
    return 0;
}

int lib_set_monrules(lua_State *L)
{
    if (server.options.monrules)
        free(server.options.monrules);

    size_t len = lua_rawlen(L, -1);
    server.options.monrule_count = len;
    server.options.monrules = calloc(len, sizeof(struct monrule));
    struct monrule *monrules = server.options.monrules;

    for (int i = 0; i < server.options.monrule_count; i++) {
        struct monrule r = get_config_array_monrule(L, "monrules", i+1);
        monrules[i] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_keybinds(lua_State *L)
{
    server.options.keybinds_ref = lua_copy_table(L);
    return 0;
}

int lib_set_buttons(lua_State *L)
{
    server.options.buttonbindings_ref = lua_copy_table(L);
    return 0;
}

int lib_set_layout_constraints(lua_State *L)
{
    server.options.layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_constraints(lua_State *L)
{
    server.options.master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);
    return 0;
}

int lib_set_update_function(lua_State *L)
{
    server.options.update_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int lib_set_resize_direction(lua_State *L)
{
    server.options.resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_master_layout_data(lua_State *L)
{
    if (lua_islayout_data(L, "master_layout_data"))
        server.options.master_layout_data_ref = lua_copy_table(L);
    else
        lua_pop(L, 1);
    return 0;
}

int lib_set_resize_data(lua_State *L)
{
    if (lua_istable(L, -1))
        server.options.resize_data_ref = lua_copy_table(L);
    else
        lua_pop(L, 1);
    return 0;
}
