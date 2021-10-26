#include "lib/lib_actions.h"

#include <inttypes.h>
#include <lua.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/backend/multi.h>

#include "container.h"
#include "ipc-server.h"
#include "layout_set.h"
#include "list_sets/container_stack_set.h"
#include "monitor.h"
#include "popup.h"
#include "root.h"
#include "server.h"
#include "stringop.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "utils/stringUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"
#include "xdg-shell-protocol.h"
#include "scratchpad.h"
#include "tagset.h"
#include "lib/lib_container.h"
#include "lib/lib_workspace.h"
#include "lib/lib_layout.h"

static const struct luaL_Reg action_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg action_f[] =
{
    {"arrange", lib_arrange},
    {"async_execute", lib_async_execute},
    {"create_output", lib_create_output},
    {"exec", lib_exec},
    {"focus_container", lib_focus_container},
    {"focus_on_hidden_stack", lib_focus_on_hidden_stack},
    {"focus_on_stack", lib_focus_on_stack},
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
    {"toggle_floating", lib_toggle_floating},
    {"toggle_layout", lib_toggle_layout},
    {"toggle_tags", lib_toggle_tags},
    {"toggle_view", lib_toggle_view},
    {"toggle_workspace", lib_toggle_workspace},
    {"view", lib_view},
    {"zoom", lib_zoom},
    {NULL, NULL},
};

static const struct luaL_Reg action_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg action_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg action_getter[] =
{
    {NULL, NULL},
};

void lua_load_action(lua_State *L)
{
    create_class(L,
            action_meta,
            action_f,
            action_m,
            action_setter,
            action_getter,
            CONFIG_ACTION);

    luaL_newlib(L, action_f);
    lua_setglobal(L, "action");
}

int lib_arrange(lua_State *L)
{
    arrange();
    return 0;
}

static void *_call(void *arg)
{
    lua_State *thread = lua_newthread(L);
    int *ref = arg;

    lua_rawgeti(thread, LUA_REGISTRYINDEX, *ref);
    lua_call_safe(thread, 0, 0, 0);

    luaL_unref(thread, LUA_REGISTRYINDEX, *ref);
    free(ref);
    return NULL;
}

int lib_async_execute(lua_State *L)
{
    pthread_t thread;
    int *func_ref = calloc(1, sizeof(int));
    *func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    pthread_create(&thread, NULL, _call, func_ref);
    pthread_detach(thread);
    return 0;
}

int lib_create_output(lua_State *L)
{
    if (!wlr_backend_is_multi(server.backend)) {
        lua_pushstring(L, "Expected a multi backend");
        return 1;
    }

    bool done = false;
    wlr_multi_for_each_backend(server.backend, create_output, &done);

    if (!done) {
        lua_pushstring(L, "Can only create outputs for Wayland, X11 or headless backends");
        return 1;
    }

    return 0;
}

int lib_focus_container(lua_State *L)
{
    int pos = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct container *con = get_container_from_container_stack_position(pos);

    if (!con)
        return 0;

    focus_container(con);
    return 0;
}

int lib_resize_main(lua_State *L)
{
    float n = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);
    enum wlr_edges dir = lt->options->resize_dir;

    lua_rawgeti(L, LUA_REGISTRYINDEX, lt->lua_resize_function_ref);
    create_lua_layout(L, lt);
    lua_pushnumber(L, n);
    lua_pushinteger(L, dir);

    if (lua_call_safe(L, 3, 1, 0) != LUA_OK) {
        return 0;
    }

    lua_copy_table_safe(L, &lt->lua_layout_copy_data_ref);

    for (int i = 0; i < lt->linked_layouts->len; i++) {
        const char *linked_layout_name = g_ptr_array_index(lt->linked_layouts, i);
        guint j;
        bool found = g_ptr_array_find_with_equal_func(ws->loaded_layouts, linked_layout_name, cmp_layout_to_string, &j);
        if (found) {
            struct layout *loc_lt = g_ptr_array_index(ws->loaded_layouts, j);
            lua_rawgeti(L, LUA_REGISTRYINDEX, loc_lt->lua_resize_function_ref);
            create_lua_layout(L, loc_lt);
            lua_pushnumber(L, n);
            lua_pushinteger(L, dir);

            lua_call_safe(L, 3, 1, 0);

            lua_copy_table_safe(L, &loc_lt->lua_layout_copy_data_ref);
        } 
    }

    arrange();
    return 0;
}

int lib_set_floating(lua_State *L)
{
    bool floating = lua_toboolean(L, -1);
    lua_pop(L, 1);
    struct monitor *m = server_get_selected_monitor();
    struct container *sel = monitor_get_focused_container(m);
    if (!sel)
        return 0;

    container_set_floating(sel, container_fix_position, floating);

    arrange();
    return 0;
}

int lib_show_scratchpad(lua_State *L)
{
    show_scratchpad();
    return 0;
}

int lib_start_keycombo(lua_State *L)
{
    const char *key_combo_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    g_ptr_array_add(server.named_key_combos, strdup(key_combo_name));
    return 0;
}

int lib_set_nmaster(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct layout *lt = get_layout_in_monitor(m);
    lt->n_master = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    arrange();
    return 0;
}

int lib_exec(lua_State *L)
{
    const char *cmd = luaL_checkstring(L, -1);
    exec(cmd);
    return 0;
}

int lib_focus_on_stack(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    focus_on_stack(m, i);
    return 0;
}

int lib_focus_on_hidden_stack(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = server_get_selected_monitor();
    focus_on_hidden_stack(m, i);
    return 0;
}

int lib_move_resize(lua_State *L)
{
    struct seat *seat = input_manager_get_default_seat();
    int ui = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    move_resize(seat->cursor, ui);
    return 0;
}

int lib_move_to_scratchpad(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);
    move_to_scratchpad(con, 0);
    return 0;
}

int lib_view(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    tagset_focus_tags(ws, ws->workspaces);
    return 0;
}

int lib_toggle_view(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    focus_most_recent_container();
    struct container *sel = monitor_get_focused_container(m);
    lift_container(sel);
    arrange(false);
    return 0;
}

int lib_toggle_floating(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct container *sel = monitor_get_focused_container(m);
    if (!sel)
        return 0;
    container_set_floating(sel, container_fix_position, !container_is_floating(sel));
    arrange();
    return 0;
}

int lib_zoom(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();

    struct container *sel = monitor_get_focused_container(m);

    if (!sel)
        return 0;

    struct workspace *ws = monitor_get_active_workspace(m);
    GPtrArray *tiled_containers = workspace_get_tiled_list_copy(ws);
    guint position;
    bool found = g_ptr_array_find(tiled_containers, sel, &position);
    if (!found)
        return 0;

    if (sel == g_ptr_array_index(tiled_containers, 0)) {
        repush(1, 0);
    } else {
        repush(position, 0);
    }
    g_ptr_array_unref(tiled_containers);

    arrange();

    // focus new master window
    struct container *con = get_container_in_stack(ws, 0);
    focus_container(con);

    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options->arrange_by_focus) {
        focus_most_recent_container();
        arrange();
    }
    return 0;
}

int lib_repush(lua_State *L)
{
    int abs_index = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    repush(i, abs_index);
    return 0;
}

int lib_swap_on_hidden_stack(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct monitor *m = server_get_selected_monitor();
    swap_on_hidden_stack(m, i);
    return 0;
}

int lib_load_next_layout_in_set(lua_State *L)
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

int lib_load_prev_layout_in_set(lua_State *L)
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

int lib_load_layout_in_set(lua_State *L)
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

int lib_load_layout(lua_State *L)
{
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    push_layout(ws, layout_name);

    arrange();
    return 0;
}

int lib_kill(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    if (!con)
        return 0;

    struct client *c = con->client;
    kill_client(c);
    return 0;
}

int lib_toggle_layout(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    push_layout(ws, ws->previous_layout);
    arrange();
    return 0;
}

int lib_toggle_tags(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    BitSet *bitset = bitset_copy(ws->prev_workspaces);
    tagset_set_tags(ws, bitset);
    debug_print("\n toggle tags: \n");
    print_bitset(ws->prev_workspaces);
    return 0;
}

int lib_toggle_workspace(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct workspace *prev_ws = get_workspace(server.previous_workspace);
    tagset_focus_workspace(m, prev_ws);
    return 0;
}

