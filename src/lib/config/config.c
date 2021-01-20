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

int lib_set_borderpx(lua_State *L)
{
    server.options.border_px = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return 0;
}

int lib_set_focus_color(lua_State *L)
{
    lua_get_color(server.options.focus_color);
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
    lua_get_color(server.options.border_color);
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    lua_get_color(server.options.root_color);
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
    return 0;
}

int lib_set_workspaces(lua_State *L)
{
    size_t len = lua_rawlen(L, -1);

    wlr_list_clear(&server.options.tag_names);
    for (int i = 1; i <= len; i++)
        wlr_list_push(&server.options.tag_names, get_config_array_str(L, "workspaces", i));
    lua_pop(L, 1);
    return 0;
}

int lib_set_rules(lua_State *L)
{
    size_t len = lua_rawlen(L, -1);
    server.options.rule_count = len;
    server.options.rules = calloc(len, sizeof(struct rule));
    struct rule *rules = server.options.rules;

    for (int i = 1; i <= len; i++) {
        struct rule r = get_config_array_rule(L, "rules", i);
        rules[i-1] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_layouts(lua_State *L)
{
    return 0;
}

int lib_set_monrules(lua_State *L)
{
    size_t len = lua_rawlen(L, -1);
    server.options.monrules = calloc(server.options.monrule_count, sizeof(struct monrule));
    struct monrule *monrules = server.options.monrules;
    server.options.monrule_count = len;

    for (int i = 1; i <= server.options.monrule_count; i++) {
        struct monrule r = get_config_array_monrule(L, "monrules", i);
        monrules[i-1] = r;
    }

    lua_pop(L, 1);
    return 0;
}

int lib_set_keybinds(lua_State *L)
{
    return 0;
}

int lib_set_buttons(lua_State *L)
{
    return 0;
}
