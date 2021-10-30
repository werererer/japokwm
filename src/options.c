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
#include "color.h"

GPtrArray *create_tagnames()
{
    GPtrArray *tag_names = g_ptr_array_new();
    return tag_names;
}

struct options *create_options()
{
    struct options *options = calloc(1, sizeof(*options));

    options->tag_names = create_tagnames();
    options->keybindings = g_ptr_array_new_with_free_func(destroy_keybinding0);
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

    free(options);
}

static void set_default_layout_set(struct options *options)
{
    // create a table looking like this: {"tile", "monocle"}
    lua_createtable(L, 0, 0);
    lua_pushstring(L, "tile");
    lua_rawseti(L, -2, 1);
    lua_pushstring(L, "monocle");
    lua_rawseti(L, -2, 2);

    int *layout_set_ref = &server.layout_set.layout_sets_ref;
    lua_copy_table_safe(L, layout_set_ref);
    const char *layout_set_key = "default";

    lua_rawgeti(L, LUA_REGISTRYINDEX, *layout_set_ref);
    lua_set_layout_set_element(L, layout_set_key, *layout_set_ref);
    lua_pop(L, 1);
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

    struct color root_color = (struct color) {
        .red = 0.3f,
        .green = 0.3f,
        .blue = 0.3f,
        .alpha = 1.0f,
    };
    struct color focus_color = RED;
    struct color border_color = BLUE;

    options->root_color = root_color;
    options->focus_color = focus_color;
    options->border_color = border_color;

    options->key_combo_timeout = 1000;
    options->repeat_rate = 25;
    options->repeat_delay = 600;
    options->tile_border_px = 3;
    options->float_border_px = 3;
    options->inner_gap = 0;
    options->outer_gap = 0;
    options->modkey = 0;
    options->arrange_by_focus = false;
    options->hidden_edges = WLR_EDGE_NONE;
    options->smart_hidden_edges = false;
    options->sloppy_focus = true;
    options->automatic_workspace_naming = true;

    list_clear(options->mon_rules, NULL);
    list_clear(options->rules, NULL);
    list_clear(options->tag_names, NULL);
    load_default_keybindings(options);

    set_default_layout_set(options);
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

static void add_keybind(struct options *options, const char *binding, const char *command)
{
    struct keybinding *keybinding = create_keybind(binding, command);
    options_add_keybinding(options, keybinding);
}

void options_add_keybinding(struct options *options, struct keybinding *keybinding)
{
    GPtrArray *keybindings = options->keybindings;
    char *sorted_binding = sort_keybinding_element(options, keybinding->binding);
    free(keybinding->binding);
    keybinding->binding = sorted_binding;

    // remove duplicates
    struct keybinding **base = (struct keybinding **)keybindings->pdata;
    int lb = lower_bound(
            &keybinding,
            base,
            keybindings->len,
            sizeof(struct keybinding *),
            cmp_keybinding);
    if (lb >= 0) {
        struct keybinding *kb = g_ptr_array_index(options->keybindings, lb);
        if (strcmp(kb->binding, sorted_binding) == 0) {
            g_ptr_array_remove_index(options->keybindings, lb);
        }
    }

    int insert_position = 1 + lb;
    g_ptr_array_insert(keybindings, insert_position, keybinding);
}

#define bind_key(options, binding, lua_func) add_keybind(options, binding, #lua_func)

void load_default_keybindings(struct options *options)
{
    list_clear(options->keybindings, NULL);

    bind_key(options, "mod-S-q", server:quit());
    bind_key(options, "mod-r", opt.reload());
    bind_key(options, "mod-S-c", 
            if Container.get_focused() then
                    Container.get_focused():kill()
            end
            );
    bind_key(options, "mod-S-Return", action.exec("/usr/bin/alacritty"));
    bind_key(options, "mod-j", action.focus_on_stack(1));
    bind_key(options, "mod-k", action.focus_on_stack(-1));
    bind_key(options, "mod-Return", action.zoom());
    bind_key(options, "mod-S-h", action.resize_main(-1/10));
    bind_key(options, "mod-S-l", action.resize_main(1/10));
    bind_key(options, "mod-p", action.exec("rofi -show run"));

    bind_key(options, "mod-S-1", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(1)
            end
            );
    bind_key(options, "mod-S-2", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(2)
            end
            );
    bind_key(options, "mod-S-3", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(3)
            end
            );
    bind_key(options, "mod-S-4", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(4)
            end
            );
    bind_key(options, "mod-S-5", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(5)
            end
            );
    bind_key(options, "mod-S-6", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(6)
            end
            );
    bind_key(options, "mod-S-7", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(7)
            end
            );
    bind_key(options, "mod-S-8", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(8)
            end
            );
    bind_key(options, "mod-S-9", 
            if Container.get_focused() then
                Container.get_focused().workspace = Workspace.get(9)
            end
            );

    bind_key(options, "mod-C-1", Workspace.get_focused().tags:_xor(1 << 0));
    bind_key(options, "mod-C-2", Workspace.get_focused().tags:_xor(1 << 1));
    bind_key(options, "mod-C-3", Workspace.get_focused().tags:_xor(1 << 2));
    bind_key(options, "mod-C-4", Workspace.get_focused().tags:_xor(1 << 3));
    bind_key(options, "mod-C-5", Workspace.get_focused().tags:_xor(1 << 4));
    bind_key(options, "mod-C-6", Workspace.get_focused().tags:_xor(1 << 5));
    bind_key(options, "mod-C-7", Workspace.get_focused().tags:_xor(1 << 6));
    bind_key(options, "mod-C-8", Workspace.get_focused().tags:_xor(1 << 7));
    bind_key(options, "mod-C-9", Workspace.get_focused().tags:_xor(1 << 8));

    bind_key(options, "mod-C-S-1", Container.get_focused():toggle_add_sticky(1 << 0));
    bind_key(options, "mod-C-S-2", Container.get_focused():toggle_add_sticky(1 << 1));
    bind_key(options, "mod-C-S-3", Container.get_focused():toggle_add_sticky(1 << 2));
    bind_key(options, "mod-C-S-4", Container.get_focused():toggle_add_sticky(1 << 3));
    bind_key(options, "mod-C-S-5", Container.get_focused():toggle_add_sticky(1 << 4));
    bind_key(options, "mod-C-S-6", Container.get_focused():toggle_add_sticky(1 << 5));
    bind_key(options, "mod-C-S-7", Container.get_focused():toggle_add_sticky(1 << 6));
    bind_key(options, "mod-C-S-8", Container.get_focused():toggle_add_sticky(1 << 7));

    bind_key(options, "mod-s 1", Workspace.get_focused():swap(Workspace.get(1)));
    bind_key(options, "mod-s 2", Workspace.get_focused():swap(Workspace.get(2)));
    bind_key(options, "mod-s 3", Workspace.get_focused():swap(Workspace.get(3)));
    bind_key(options, "mod-s 4", Workspace.get_focused():swap(Workspace.get(4)));
    bind_key(options, "mod-s 5", Workspace.get_focused():swap(Workspace.get(5)));
    bind_key(options, "mod-s 6", Workspace.get_focused():swap(Workspace.get(6)));
    bind_key(options, "mod-s 7", Workspace.get_focused():swap(Workspace.get(7)));
    bind_key(options, "mod-s 8", Workspace.get_focused():swap(Workspace.get(8)));

    bind_key(options, "mod-t", Container.get_focused().floating = false);

    bind_key(options, "mod-M1", action.move_resize(Cursor_mode.move));
    bind_key(options, "mod-M2", action.move_resize(Cursor_mode.resize));
    bind_key(options, "M1", info.get_container_under_cursor():focus());

    bind_key(options, "mod-a", Layout.get_focused():increase_n_master());
    bind_key(options, "mod-x", Layout.get_focused():decrease_n_master());

    bind_key(options, "mod-1", action.view_or_tag(1));
    bind_key(options, "mod-2", action.view_or_tag(2));
    bind_key(options, "mod-3", action.view_or_tag(3));
    bind_key(options, "mod-4", action.view_or_tag(4));
    bind_key(options, "mod-5", action.view_or_tag(5));
    bind_key(options, "mod-6", action.view_or_tag(6));
    bind_key(options, "mod-7", action.view_or_tag(7));
    bind_key(options, "mod-8", action.view_or_tag(8));
    bind_key(options, "mod-9", action.view_or_tag(9));

    bind_key(options, "mod-b", Workspace.get_focused():toggle_bars());
    bind_key(options, "mod-m", Workspace.get_focused().stack[1]:focus());

    bind_key(options, "mod-S-j", action.focus_on_hidden_stack(0));
    bind_key(options, "mod-S-k", action.focus_on_hidden_stack(-1));

    bind_key(options, "mod-e", action.view(Workspace.get_focused():get_next_empty(Direction.left)));
    bind_key(options, "mod-S-space", Layout.load_prev_in_set("default"));
    bind_key(options, "mod-space", Layout.load_next_in_set("default"));

    bind_key(options, "mod-minus", action.move_to_scratchpad(info.this_container_position()));
    bind_key(options, "mod-S-minus", action.show_scratchpad());

    bind_key(options, "mod-tab", action.swap_on_hidden_stack(0));
    bind_key(options, "mod-S-tab", action.swap_on_hidden_stack(-1));

    bind_key(options, "mod-period", action.toggle_workspace());
    bind_key(options, "mod-S-period", action.toggle_tags());

    bind_key(options, "mod-0",
            Workspace.get_focused().tags = 1 << Workspace.get_focused():get_id()
        );
    bind_key(options, "mod-S-0",
        local con = Container.get_focused()
        if con then
            Workspace.get_focused().tags = 1 << con.workspace:get_id()
        end
    );
    bind_key(options, "mod-C-S-0",
            local con = Container.get_focused()
            if con then
                con.sticky_restricted = 0
            end
            );
    bind_key(options, "mod-C-S-9",
            local con = Container.get_focused()
            if con then
                con.sticky_restricted = 255
            end
            );
}

static void assign_list(
        GPtrArray **dest_arr,
        GPtrArray *src_arr,
        GCopyFunc copy_func)
{
    g_ptr_array_unref(*dest_arr);
    *dest_arr = g_ptr_array_copy(src_arr, copy_func, NULL);
}

void copy_options(struct options *dest_option, struct options *src_option)
{
    dest_option->root_color = src_option->root_color;
    dest_option->focus_color = src_option->focus_color;
    dest_option->border_color = src_option->border_color;

    dest_option->resize_dir = src_option->resize_dir;
    dest_option->layout_constraints = src_option->layout_constraints;
    dest_option->master_constraints = src_option->master_constraints;
    dest_option->sloppy_focus = src_option->sloppy_focus;
    dest_option->key_combo_timeout = src_option->key_combo_timeout;
    dest_option->repeat_rate = src_option->repeat_rate;
    dest_option->repeat_delay = src_option->repeat_delay;
    dest_option->tile_border_px = src_option->tile_border_px;
    dest_option->float_border_px = src_option->float_border_px;
    dest_option->inner_gap = src_option->inner_gap;
    dest_option->outer_gap = src_option->outer_gap;
    dest_option->modkey = src_option->modkey;
    dest_option->arrange_by_focus = src_option->arrange_by_focus;
    dest_option->hidden_edges = src_option->hidden_edges;
    dest_option->smart_hidden_edges = src_option->smart_hidden_edges;
    dest_option->automatic_workspace_naming = src_option->automatic_workspace_naming;

    assign_list(&dest_option->mon_rules, src_option->mon_rules, NULL);
    assign_list(&dest_option->rules, src_option->rules, NULL);
    assign_list(&dest_option->tag_names, src_option->tag_names, NULL);
    assign_list(&dest_option->keybindings, src_option->keybindings, copy_keybinding);
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
