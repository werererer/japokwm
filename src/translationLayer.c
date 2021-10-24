#include "translationLayer.h"

#include <lauxlib.h>
#include <lua.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_output_layout.h>

#include "cursor.h"
#include "lib/lib_actions.h"
#include "lib/lib_bitset.h"
#include "lib/lib_color.h"
#include "lib/lib_container.h"
#include "lib/lib_event_handler.h"
#include "lib/lib_geom.h"
#include "lib/lib_info.h"
#include "lib/lib_layout.h"
#include "lib/lib_list.h"
#include "lib/lib_list2D.h"
#include "lib/lib_monitor.h"
#include "lib/lib_options.h"
#include "lib/lib_server.h"
#include "lib/lib_workspace.h"
#include "lib/local_event_handler.h"
#include "lib/local_options.h"
#include "lib/lib_focus_set.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "workspace.h"

const struct luaL_Reg meta[] = 
{
    {"__index", get_lua_value},
    {"__newindex", set_lua_value},
    {NULL, NULL},
};


static const struct luaL_Reg action[] =
{
    {"arrange", lib_arrange},
    {"async_execute", lib_async_execute},
    {"create_output", lib_create_output},
    {"decrease_nmaster", lib_decrease_nmaster},
    {"exec", lib_exec},
    {"focus_container", lib_focus_container},
    {"focus_on_hidden_stack", lib_focus_on_hidden_stack},
    {"focus_on_stack", lib_focus_on_stack},
    {"increase_nmaster", lib_increase_nmaster},
    {"kill", lib_kill},
    {"load_layout", lib_load_layout},
    {"load_layout_in_set", lib_load_layout_in_set},
    {"load_next_layout_in_set", lib_load_next_layout_in_set},
    {"load_prev_layout_in_set", lib_load_prev_layout_in_set},
    {"move_resize", lib_move_resize},
    {"move_to_scratchpad", lib_move_to_scratchpad},
    {"repush", lib_repush},
    {"resize_main", lib_resize_main},
    {"set_floating", lib_set_floating},
    {"set_nmaster", lib_set_nmaster},
    {"show_scratchpad", lib_show_scratchpad},
    {"start_keycombo", lib_start_keycombo},
    {"swap_on_hidden_stack", lib_swap_on_hidden_stack},
    {"toggle_bars", lib_toggle_bars},
    {"toggle_floating", lib_toggle_floating},
    {"toggle_layout", lib_toggle_layout},
    {"toggle_tags", lib_toggle_tags},
    {"toggle_view", lib_toggle_view},
    {"toggle_workspace", lib_toggle_workspace},
    {"view", lib_view},
    {"zoom", lib_zoom},
    {NULL, NULL},
};

static const struct luaL_Reg info[] =
{
    {"get_active_layout", lib_get_active_layout},
    {"get_container_under_cursor", lib_get_container_under_cursor},
    {"get_n_tiled", lib_get_n_tiled},
    {"get_next_empty_workspace", lib_workspace_get_next_empty},
    {"get_nmaster", lib_get_nmaster},
    {"get_previous_layout", lib_get_previous_layout},
    {"get_root_area", lib_get_root_area},
    {"get_this_container_count", lib_get_this_container_count},
    {"get_workspace_count", lib_get_workspace_count},
    {"is_container_not_in_limit", lib_is_container_not_in_limit},
    {"is_container_not_in_master_limit", lib_is_container_not_in_master_limit},
    {"is_keycombo", lib_is_keycombo},
    {"stack_position_to_position", lib_stack_position_to_position},
    {"this_container_position", lib_this_container_position},
    {NULL, NULL},
};

static const struct luaL_Reg monitor[] = 
{
    {"set_scale", lib_monitor_set_scale},
    {"set_transform", lib_monitor_set_transform},
    {NULL, NULL},
};

static void load_info(lua_State *L)
{
    luaL_newlib(L, info);

    lua_createtable(L, 0, 0);
    lua_createtable(L, 0, 0);

    lua_pushinteger(L, CURSOR_NORMAL);
    lua_setfield(L, -2 ,"normal");

    lua_pushinteger(L, CURSOR_MOVE);
    lua_setfield(L, -2 ,"move");

    lua_pushinteger(L, CURSOR_RESIZE);
    lua_setfield(L, -2 ,"resize");

    lua_setfield(L, -2 ,"mode");
    lua_setfield(L, -2 ,"cursor");

    lua_createtable(L, 0, 0);

    lua_createtable(L, 0, 0);

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_NORMAL);
    lua_setfield(L, -2 ,"normal");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_90);
    lua_setfield(L, -2 ,"rotate_90");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_180);
    lua_setfield(L, -2 ,"rotate_180");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_270);
    lua_setfield(L, -2 ,"rotate_270");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED);
    lua_setfield(L, -2 ,"flipp");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED_90);
    lua_setfield(L, -2 ,"flipp_90");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED_180);
    lua_setfield(L, -2 ,"flipp_180");

    lua_pushinteger(L, WL_OUTPUT_TRANSFORM_FLIPPED_270);
    lua_setfield(L, -2 ,"flipp_270");

    lua_setfield(L, -2 ,"transform");

    lua_setfield(L, -2 ,"monitor");

    lua_createtable(L, 0, 0);

    lua_pushinteger(L, 0);
    lua_setfield(L, -2, "none");

    lua_pushinteger(L, WLR_DIRECTION_UP | WLR_DIRECTION_DOWN |
            WLR_DIRECTION_LEFT | WLR_DIRECTION_RIGHT);
    lua_setfield(L, -2, "all");

    lua_pushinteger(L, WLR_EDGE_BOTTOM);
    lua_setfield(L, -2, "bottom");

    lua_pushinteger(L, WLR_EDGE_TOP);
    lua_setfield(L, -2, "top");

    lua_pushinteger(L, WLR_EDGE_LEFT);
    lua_setfield(L, -2, "left");

    lua_pushinteger(L, WLR_EDGE_RIGHT);
    lua_setfield(L, -2, "right");

    lua_pushinteger(L, WLR_EDGE_NONE);
    lua_setfield(L, -2, "none");

    lua_setfield(L, -2, "direction");

    lua_setglobal(L, "info");
}

/* set a lua value
 * */
int set_lua_value(lua_State *L)
{
    // [table, key, value]
    const char *key = luaL_checkstring(L, -2);

    lua_getmetatable(L, -3); {
        // [table, key, value, meta]
        lua_pushstring(L, "setter");
        // [table, key, value, meta, "setter"]
        lua_gettable(L, -2);
        // [table, key, value, meta, (table)meta."setter"]
        lua_pushstring(L, key);
        // [table, key, value, meta, (table)meta."setter", key]
        lua_gettable(L, -2);
        // [table, key, value, meta, (table)meta."setter", (value)meta."setter".key]
        if (lua_isnil(L, -1)) {
            lua_pop(L, 6);

            luaL_where(L, 1);
            const char *where = luaL_checkstring(L, -1);
            char *res = g_strconcat(
                    where,
                    key,
                    " can't be set. Adding new values to lib tables is illegal",
                    NULL);
            // from here on using the variable that was on the stack may
            // lead to segfaults because lua may garbage collect them
            lua_pop(L, 1);
            lua_pushstring(L, res);
            lua_warning(L, res, false);
            free(res);
            // lua_error(L);
            return 0;
        }
        lua_pop(L, 2);
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

/* this is very bad code I know... I returns the metatable after going through
 * the getter function */
int get_lua_value(lua_State *L)
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

void load_lua_api(lua_State *L)
{
    luaL_newlib(L, action);
    lua_setglobal(L, "action");

    luaL_openlibs(L);

    load_info(L);

    lua_load_bitset();
    lua_load_focus_set();
    lua_load_monitor();
    lua_load_color();
    lua_load_container();
    lua_load_events();
    lua_load_geom();
    lua_load_layout();
    lua_load_list();
    lua_load_list2D();
    lua_load_options();
    lua_load_server();
    lua_load_workspace();

    luaL_newlib(L, monitor);
    lua_setglobal(L, "monitor");
}

void init_global_config_variables(lua_State *L)
{
    lua_init_events(server.event_handler);
    lua_init_options(server.default_layout->options);
    lua_init_layout(server.default_layout);
}

void init_local_config_variables(lua_State *L, struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);

    lua_init_events(server.event_handler);
    lua_init_options(lt->options);
    lua_init_layout(lt);
}
