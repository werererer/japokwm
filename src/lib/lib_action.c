#include "lib/lib_action.h"

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
#include "tag.h"
#include "xdg-shell-protocol.h"
#include "scratchpad.h"
#include "tagset.h"
#include "lib/lib_container.h"
#include "lib/lib_tag.h"
#include "lib/lib_layout.h"
#include "layer_shell.h"
#include "root.h"

static const struct luaL_Reg action_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg action_static_methods[] =
{
    {"arrange", lib_arrange},
    {"create_output", lib_create_output},
    {"deep_copy", lib_deep_copy},
    {"exec", lib_exec},
    {"focus_on_hidden_stack", lib_focus_on_hidden_stack},
    {"focus_on_stack", lib_focus_on_stack},
    {"move_resize", lib_move_resize},
    {"move_to_scratchpad", lib_move_to_scratchpad},
    {"resize_main", lib_resize_main},
    {"show_scratchpad", lib_show_scratchpad},
    {"start_keycombo", lib_start_keycombo},
    {"swap_on_hidden_stack", lib_swap_on_hidden_stack},
    {"toggle_all_bars", lib_toggle_all_bars},
    {"toggle_tags", lib_toggle_tags},
    {"toggle_view", lib_toggle_view},
    {"toggle_tag", lib_toggle_tag},
    {"view", lib_view},
    {"view_or_tag", lib_view_or_tag},
    {"resize_with_cursor", lib_resize_with_cursor},
    {"zoom", lib_zoom},
    {NULL, NULL},
};

static const struct luaL_Reg action_methods[] =
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
            action_static_methods,
            action_methods,
            action_setter,
            action_getter,
            CONFIG_ACTION);

    luaL_newlib(L, action_static_methods);
    lua_setglobal(L, "Action");
}

int lib_arrange(lua_State *L)
{
    arrange();
    return 0;
}

static void *_call(void *arg)
{
    struct function_data *data = arg;

    char path[1024];
    const char *cmd = data->cmd;
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
        return NULL;
    }

    data->output = strdup("");
    /* Read the output a line at a time - output it. */
    while (fgets(path, sizeof(path), fp) != NULL) {
        append_string(&data->output, path);
        printf("%s", path);
    }

    /* close */
    pclose(fp);

    server.async_handler.data = data;
    if (data->lua_func_ref == -1) {
        return NULL;
    }
    uv_async_send(&server.async_handler);

    return NULL;
}

int lib_exec(lua_State *L)
{
    pthread_t thread;
    struct function_data *data = malloc(sizeof(struct function_data));

    if (lua_gettop(L) == 2) {
        lua_isfunction(L, -1);
        data->lua_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
        data->lua_func_ref = -1;
    }

    data->cmd = strdup(luaL_checkstring(L, 1));
    lua_pop(L, 1);
    data->L = L;

    pthread_create(&thread, NULL, _call, data);
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

int lib_deep_copy(lua_State *L)
{
    if (lua_gettop(L) != 1) {
        luaL_error(L, "function expects exactly one argument");
    }
    lua_pushcfunction(L, deep_copy_table);
    lua_insert(L, -2);
    lua_call_safe(L, 1, 1, 0);
    return 1;
}

int lib_resize_main(lua_State *L)
{
    float n = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    struct layout *lt = tag_get_layout(tag);
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
        bool found = g_ptr_array_find_with_equal_func(tag->loaded_layouts, linked_layout_name, cmp_layout_to_string, &j);
        if (found) {
            struct layout *loc_lt = g_ptr_array_index(tag->loaded_layouts, j);
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

int lib_show_scratchpad(lua_State *L)
{
    show_scratchpad();
    return 0;
}

int lib_start_keycombo(lua_State *L)
{
    const char *key_combo_name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    server_start_keycombo(key_combo_name);
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
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);
    move_to_scratchpad(con, 0);
    return 0;
}

int lib_view(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    struct monitor *old_sel_m = server_get_selected_monitor();

    tagset_focus_tags(tag, tag->tags);
    struct monitor *m = tag_get_selected_monitor(tag);

    // we only want to center the cursor if the monitor changes
    if (m != old_sel_m) {
        server_center_default_cursor_in_monitor(m);
    }
    return 0;
}

int lib_view_or_tag(lua_State *L)
{
    int i = lua_idx_to_c_idx(luaL_checkinteger(L, 1));
    lua_pop(L, 1);

    if (server_is_keycombo("_lib_view_or_tag_combo")) {
        struct tag *tag = server_get_selected_tag();
        BitSet *bitset = bitset_create();
        bitset_set(bitset, i);
        bitset_xor(tag->tags, bitset);
        bitset_destroy(bitset);
        tagset_focus_tag(tag);
    } else {
        struct tag *tag = get_tag(i);

        struct monitor *old_sel_m = server_get_selected_monitor();
        tagset_focus_tag(tag);
        struct monitor *m = tag_get_selected_monitor(tag);

        // we only want to center the cursor if the monitor changes
        if (m != old_sel_m) {
            server_center_default_cursor_in_monitor(m);
        }
    }
    server_start_keycombo("_lib_view_or_tag_combo");
    return 0;
}

int lib_resize_with_cursor(lua_State *L)
{
    printf("resize with cursor\n");
    return 0;
}

int lib_toggle_view(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    tag_this_focus_most_recent_container();
    struct container *sel = monitor_get_focused_container(m);
    lift_container(sel);
    arrange(false);
    return 0;
}

int lib_zoom(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();

    struct container *sel = monitor_get_focused_container(m);

    if (!sel)
        return 0;

    struct tag *tag = monitor_get_active_tag(m);
    GPtrArray *tiled_containers = tag_get_tiled_list_copy(tag);
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
    struct container *con = get_container_in_stack(tag, 0);
    tag_this_focus_container(con);

    struct layout *lt = get_layout_in_monitor(m);
    if (lt->options->arrange_by_focus) {
        tag_this_focus_most_recent_container();
        arrange();
    }
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

int lib_toggle_all_bars(lua_State *L)
{
    enum wlr_edges edges =
            WLR_EDGE_BOTTOM |
            WLR_EDGE_RIGHT |
            WLR_EDGE_LEFT |
            WLR_EDGE_TOP;
    if (lua_gettop(L) >= 2) {
        edges = luaL_checkinteger(L, 1);
        lua_pop(L, 1);
    }

    struct tag *sel_tag = server_get_selected_tag();

    // toggle edges
    enum wlr_edges visible_edges = WLR_EDGE_NONE;
    if (sel_tag->visible_bar_edges == WLR_EDGE_NONE) {
        visible_edges = edges;
    }

    for (int i = 0; i < server_get_tag_key_count(); i++) {
        struct tag *tag = get_tag(i);
        set_bars_visible(tag, visible_edges);
    }
    return 0;
}

int lib_toggle_tags(lua_State *L)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    BitSet *prev_tags_copy = server_bitset_get_tmp_copy(tag->prev_tags);
    tagset_set_tags(tag, prev_tags_copy);
    return 0;
}

int lib_toggle_tag(lua_State *L)
{
    struct tag *prev_tag = get_tag(server.previous_tag);
    tagset_focus_tag(prev_tag);
    return 0;
}
