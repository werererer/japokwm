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

struct options *create_options()
{
    struct options *options = calloc(1, sizeof(*options));

    options->tag_names = create_tagnames();
    options->keybindings = g_ptr_array_new();
    options->mon_rules = g_ptr_array_new();
    options->rules = g_ptr_array_new();

    options_reset(options);

    return options;
}

void destroy_options(struct options *options)
{
    g_ptr_array_unref(options->tag_names);
    g_ptr_array_unref(options->keybindings);
    g_ptr_array_unref(options->mon_rules);
    g_ptr_array_unref(options->rules);
}

void options_reset(struct options *options)
{
    options->resize_dir = 0;
    options->layout_constraints = (struct resize_constraints) {
        .min_width = 0.1f,
        .min_height = 0.1f,
        .max_width = 1.0f,
        .max_height = 1.0f
    };
    options->master_constraints = (struct resize_constraints) {
        .min_width = 0.1f,
        .min_height = 0.1f,
        .max_width = 1.0f,
        .max_height = 1.0f
    };

    float root_color[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    float focus_color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float border_color[4] = {0.0f, 0.0f, 1.0f, 1.0f};

    memcpy(options->root_color, root_color, sizeof(float)*4);
    memcpy(options->focus_color, focus_color, sizeof(float)*4);
    memcpy(options->border_color, border_color, sizeof(float)*4);

    options->key_combo_timeout = 1000;
    options->repeat_rate = 25;
    options->repeat_delay = 600;
    options->tile_border_px = 3;
    options->float_border_px = 3;
    options->inner_gap = 0;
    options->outer_gap = 0;
    options->event_handler = create_event_handler();
    options->modkey = 0;
    options->arrange_by_focus = false;
    options->hidden_edges = WLR_EDGE_NONE;
    options->smart_hidden_edges = false;
    options->automatic_workspace_naming = false;

    list_clear(options->mon_rules, NULL);
    list_clear(options->rules, NULL);
    list_clear(options->tag_names, NULL);
    list_clear(options->keybindings, NULL);
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
    struct options *options = lt->options;
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

static void assign_list(GPtrArray **dest_arr, GPtrArray *src_arr)
{
    g_ptr_array_unref(*dest_arr);
    *dest_arr = g_ptr_array_copy(src_arr, NULL, NULL);
}

void copy_options(struct options *dest_option, struct options *src_option)
{
    dest_option->tile_border_px = src_option->tile_border_px;

    dest_option->resize_dir = src_option->resize_dir;
    dest_option->layout_constraints = src_option->layout_constraints;
    dest_option->master_constraints = src_option->master_constraints;

    memcpy(dest_option->root_color, src_option->root_color, sizeof(float)*4);
    memcpy(dest_option->focus_color, src_option->focus_color, sizeof(float)*4);
    memcpy(dest_option->border_color, src_option->border_color, sizeof(float)*4);

    dest_option->key_combo_timeout = src_option->key_combo_timeout;
    dest_option->repeat_rate = src_option->repeat_rate;
    dest_option->repeat_delay = src_option->repeat_delay;
    dest_option->tile_border_px = src_option->tile_border_px;
    dest_option->float_border_px = src_option->float_border_px;
    dest_option->inner_gap = src_option->inner_gap;
    dest_option->outer_gap = src_option->outer_gap;
    dest_option->event_handler = src_option->event_handler;
    dest_option->modkey = src_option->modkey;
    dest_option->arrange_by_focus = src_option->arrange_by_focus;
    dest_option->hidden_edges = src_option->hidden_edges;
    dest_option->smart_hidden_edges = src_option->smart_hidden_edges;
    dest_option->automatic_workspace_naming = src_option->automatic_workspace_naming;

    assign_list(&dest_option->mon_rules, src_option->mon_rules);
    assign_list(&dest_option->rules, src_option->rules);
    assign_list(&dest_option->tag_names, src_option->tag_names);
    assign_list(&dest_option->keybindings, src_option->keybindings);

    reset_floating_client_borders(dest_option->tile_border_px);
}

int workspace_get_new_position(struct workspace *ws)
{
    struct layout *lt = workspace_get_layout(ws);
    int func_ref = lt->options->new_position_func_ref;
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
    int func_ref = lt->options->new_focus_position_func_ref;
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
