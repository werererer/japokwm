#include "options.h"

#include <string.h>
#include <glib.h>

#include "client.h"
#include "monitor.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"
#include "layout.h"
#include "keybinding.h"
#include "workspace.h"

GPtrArray *create_tagnames()
{
    GPtrArray *tag_names = g_ptr_array_new();
    g_ptr_array_add(tag_names, "1:1");
    g_ptr_array_add(tag_names, "2:2");
    g_ptr_array_add(tag_names, "3:3");
    g_ptr_array_add(tag_names, "4:4");
    g_ptr_array_add(tag_names, "5:5");
    g_ptr_array_add(tag_names, "6:6");
    g_ptr_array_add(tag_names, "7:7");
    g_ptr_array_add(tag_names, "8:8");
    g_ptr_array_add(tag_names, "9:9");
    return tag_names;
}

struct options get_default_options()
{
    struct options options = {
        .resize_dir = 0,
        .layout_constraints = {
            .min_width = 0.1f,
            .min_height = 0.1f,
            .max_width = 1.0f,
            .max_height = 1.0f
        },
        .master_constraints = {
            .min_width = 0.1f,
            .min_height = 0.1f,
            .max_width = 1.0f,
            .max_height = 1.0f
        },
        .root_color = {0.3f, 0.3f, 0.3f, 1.0f},
        .focus_color = {1.0f, 0.0f, 0.0f, 1.0f},
        .border_color = {0.0f, 0.0f, 1.0f, 1.0f},
        .key_combo_timeout = 1000,
        .repeat_rate = 25,
        .repeat_delay = 600,
        .tile_border_px = 3,
        .float_border_px = 3,
        .inner_gap = 0,
        .outer_gap = 0,
        .event_handler = create_event_handler(),
        .mon_rules = g_ptr_array_new(),
        .rules = g_ptr_array_new(),
        .modkey = 0,
        .arrange_by_focus = false,
        .hidden_edges = WLR_EDGE_NONE,
        .smart_hidden_edges = false,
        .automatic_workspace_naming = false,
    };

    options.tag_names = create_tagnames();
    options.keybindings = g_ptr_array_new_with_free_func(destroy_keybinding0);
    return options;
}

static struct keybinding *create_keybind(const char *binding, const char *command)
{
    int ref = 0;
    char *cmd = g_strconcat("return function() ", command, " end", NULL);
    luaL_dostring(L, cmd);
    lua_ref_safe(L, LUA_REGISTRYINDEX, &ref);
    free(cmd);
    struct keybinding *keybinding = create_keybinding(binding, ref);
    return keybinding;
}

static void add_keybind(GPtrArray *keybindings, const char *binding, const char *command)
{
    struct keybinding *keybinding = create_keybind(binding, command);
    g_ptr_array_add(keybindings, keybinding);
}

void load_default_keybindings()
{
    struct layout *lt = server.default_layout;
    struct options *options = &lt->options;
    GPtrArray *keybindings = options->keybindings;

    add_keybind(keybindings, "mod-S-q", "action.quit()");
    add_keybind(keybindings, "mod-r", "config.reload()");
    add_keybind(keybindings, "mod-S-c", "action.kill(info.this_container_position())");
    add_keybind(keybindings, "mod-S-Return", "action.exec(\"/usr/bin/alacritty\")");
    add_keybind(keybindings, "mod-j", "action.focus_on_stack(1)");
    add_keybind(keybindings, "mod-k", "action.focus_on_stack(-1)");
    add_keybind(keybindings, "mod-Return", "action.zoom()");
    add_keybind(keybindings, "mod-S-h", "action.resize_main(-1/10)");
    add_keybind(keybindings, "mod-S-l", "action.resize_main(1/10)");
    return;
}

void copy_options(struct options *dest_option, struct options *src_option)
{
    memcpy(dest_option, src_option, sizeof(struct options));

    reset_floating_client_borders(dest_option->tile_border_px);
}

int workspace_get_new_position(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);
    int func_ref = lt->options.new_position_func_ref;
    if (func_ref == 0) {
        return 0;
    } else {
        int ws_id = ws->id;
        bool is_focused = is_workspace_the_selected_one(ws)
            && workspace_get_monitor(ws) == server_get_selected_monitor();
        lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
        lua_pushinteger(L, ws_id);
        lua_pushboolean(L, is_focused);
        lua_call_safe(L, 2, 1, 0);
        int i = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        return i;
    }
}

int workspace_get_new_focus_position(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);
    int func_ref = lt->options.new_focus_position_func_ref;
    if (func_ref == 0) {
        return 0;
    } else {
        int ws_id = ws->id;
        bool is_focused = is_workspace_the_selected_one(ws)
            && workspace_get_monitor(ws) == server_get_selected_monitor();
        lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
        lua_pushinteger(L, ws_id);
        lua_pushboolean(L, is_focused);
        lua_call_safe(L, 2, 1, 0);
        int i = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        return i;
    }
}
