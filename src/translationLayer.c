#include "translationLayer.h"

#include <lauxlib.h>
#include <libnotify/notify.h>
#include <lua.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_output_layout.h>

#include "cursor.h"
#include "lib/lib_actions.h"
#include "lib/lib_bitset.h"
#include "lib/lib_color.h"
#include "lib/lib_container.h"
#include "lib/lib_container_property.h"
#include "lib/lib_cursor.h"
#include "lib/lib_cursor_mode.h"
#include "lib/lib_direction.h"
#include "lib/lib_event_handler.h"
#include "lib/lib_focus_set.h"
#include "lib/lib_geom.h"
#include "lib/lib_info.h"
#include "lib/lib_layout.h"
#include "lib/lib_list.h"
#include "lib/lib_list2D.h"
#include "lib/lib_monitor.h"
#include "lib/lib_options.h"
#include "lib/lib_output_transform.h"
#include "lib/lib_ring_buffer.h"
#include "lib/lib_root.h"
#include "lib/lib_server.h"
#include "lib/lib_workspace.h"
#include "lib/local_event_handler.h"
#include "lib/local_options.h"
#include "server.h"
#include "stringop.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"

const struct luaL_Reg meta[] = 
{
    {"__index", get_lua_value},
    {"__newindex", set_lua_value},
    {NULL, NULL},
};

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

// replace lua print statement with notify-send
static int l_print(lua_State *L) {
    int nargs = lua_gettop(L);

    char *msg = strdup("");
    for (int i=1; i <= nargs; i++) {
        const char *string = lua_tostring(L, i);

        append_string(&msg, string);
    }
    notify_msg(msg);
    write_line_to_error_file(msg);
    free(msg);

    return 0;
}

static const struct luaL_Reg mylib [] = {
    {"print", l_print},
    {NULL, NULL}
};

void load_lua_api(lua_State *L)
{
    luaL_openlibs(L);

    lua_getglobal(L, "_G");
    luaL_setfuncs(L, mylib, 0);
    lua_pop(L, 1);

    lua_load_action(L);
    lua_load_bitset(L);
    lua_load_color(L);
    lua_load_container(L);
    lua_load_container_property(L);
    lua_load_cursor(L);
    lua_load_cursor_mode(L);
    lua_load_direction(L);
    lua_load_events(L);
    lua_load_focus_set(L);
    lua_load_geom(L);
    lua_load_info(L);
    lua_load_layout(L);
    lua_load_list(L);
    lua_load_list2D(L);
    lua_load_monitor(L);
    lua_load_options(L);
    lua_load_output_transform(L);
    lua_load_ring_buffer(L);
    lua_load_root(L);
    lua_load_server(L);
    lua_load_workspace(L);
}

void init_global_config_variables(lua_State *L)
{
    lua_init_events(server.event_handler);
    lua_init_options(server.default_layout->options);
    lua_init_layout(server.default_layout);
}

void init_local_config_variables(lua_State *L, struct layout *lt)
{
    lua_init_events(server.event_handler);
    lua_init_options(lt->options);
    lua_init_layout(lt);
}
