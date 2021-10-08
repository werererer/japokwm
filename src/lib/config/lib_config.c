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

/* this is very bad code I know... I returns the metatable after going through
 * the getter function */
int get(lua_State *L)
{
    // [table, key]
    const char *key = luaL_checkstring(L, -1);

    // call functions if available
    lua_getmetatable(L, -2);
    // [table, key, meta]
    lua_pushstring(L, key);
    // [table, key, meta, key]
    lua_gettable(L, -2);
    // [table, key, meta, (cfunction|NULL)meta.key]
    if (!lua_isnil(L, -1)) {
        lua_insert(L, -3);
        // [(cfunction|NULL)meta.key, table, key]
        lua_pop(L, 2);
        // [(cfunction|NULL)meta.key]
        return 1;
    }
    lua_pop(L, 2);
    // [table, key]

    lua_getmetatable(L, -2);
    // [table, key, meta]

    lua_pushstring(L, "getter");
    // [table, key, meta, "getter"]
    lua_gettable(L, -2);
    // [table, key, meta, (table)meta."getter"]
    lua_pushstring(L, key);
    // [table, key, meta, (table)meta."getter", key]
    lua_gettable(L, -2);
    // [table, key, meta, (table)meta."getter", (cfunc)meta."getter".key]
    lua_pushvalue(L, -5);
    // [table, key, meta, (table)meta."getter", (cfunc)meta."getter".key, table]
    lua_pcall(L, 1, 1, 0);
    // [table, key, meta, (table)meta."getter", retval]
    lua_insert(L, -5);
    // [retval, table, key, meta, (table)meta."getter".key]
    lua_pop(L, 4);
    // [retval]
    return 1;
}

/* set a lua value
 * */
int set(lua_State *L)
{
    // [table, key, value]
    const char *key = luaL_checkstring(L, -2);

    lua_getmetatable(L, -3); {
        // [table, key, value, meta]
        lua_pushstring(L, "setter");
        // [table, key, value, meta, "setter"]
        lua_gettable(L, -2); {
            // [table, key, value, meta, (table)meta."setter"]
            lua_pushstring(L, key);
            // [table, key, value, meta, (table)meta."setter", key]
            lua_gettable(L, -2); {

                // [table, key, value, meta, (table)meta."setter",
                // (cfunction)meta."setter".key]
                lua_pushvalue(L, -6);
                // [table, key, value, meta, (table)meta."setter",
                // (cfunction)meta."setter".key, table]
                lua_pushvalue(L, -5);
                // [table, key, value, meta, (table)meta."setter",
                // (cfunction)meta."setter".key, table, value]
                lua_pcall(L, 2, 0, 0);
                // [table, key, value, meta, (table)meta."setter"]

            } lua_pop(L, 1);
            // [table, key, value, meta]

            lua_pop(L, 1);
            // [table, key, value]

        } lua_pop(L, 1);
        // [table, key]
    } lua_pop(L, 1);
    // [table]

    lua_pop(L, 1);
    // []

    return 0;
}

int lib_reload(lua_State *L)
{
    if (server_is_config_reloading_prohibited()) {
        luaL_where(L, 1);
        const char *where = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        char *res = g_strconcat(
                where,
                "action.reload() aborted: infinit recursion detected",
                NULL);
        debug_print("line: %s\n", res);
        lua_pushstring(L, res);
        lua_error(L);
    }
    server_prohibit_reloading_config();

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

static struct options *check_options(lua_State *L) {
    void **ud = luaL_checkudata(L, 1, "japokwm.config");
    luaL_argcheck(L, ud != NULL, 1, "`option' expected");
    return (struct options *)*ud;
}

int lib_get_sloppy_focus(lua_State *L)
{
    struct options *options = check_options(L);
    lua_pushinteger(L, options->sloppy_focus);
    int sloppy_focus = server.default_layout->options->sloppy_focus;
    lua_pushinteger(L, sloppy_focus);
    return 1;
}

int lib_get_inner_gaps(lua_State *L)
{
    struct options *options = check_options(L);
    int inner_gap = options->inner_gap;
    lua_pushinteger(L, inner_gap);
    return 1;
}
