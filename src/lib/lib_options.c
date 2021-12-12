#include "lib/lib_options.h"

#include "color.h"
#include "ipc-server.h"
#include "keybinding.h"
#include "lib/lib_color.h"
#include "monitor.h"
#include "rules/mon_rule.h"
#include "rules/rule.h"
#include "server.h"
#include "tagset.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "utils/coreUtils.h"
#include "utils/gapUtils.h"
#include "utils/parseConfigUtils.h"
#include "workspace.h"
#include "lib/lib_direction.h"
#include "server.h"

static const struct luaL_Reg options_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg options_f[] = 
{
    {"reload", lib_reload},
    {NULL, NULL},
};

static const struct luaL_Reg options_m[] =
{
    {"add_mon_rule", lib_add_mon_rule},
    {"add_rule", lib_add_rule},
    {"bind_key", lib_bind_key},
    {"set_layout_constraints", lib_set_layout_constraints},
    {"set_master_constraints", lib_set_master_constraints},
    {NULL, NULL},
};

static const struct luaL_Reg options_setter[] =
{
    {"arrange_by_focus", lib_set_arrange_by_focus},
    {"automatic_workspace_naming", lib_set_automatic_workspace_naming},
    {"border_color", lib_set_border_color},
    {"border_width", lib_set_tile_border_width},
    {"default_layout", lib_set_default_layout},
    {"entry_focus_position_function", lib_set_entry_focus_position_function},
    {"entry_position_function", lib_set_entry_position_function},
    {"float_border_width", lib_set_float_border_width},
    {"focus_color", lib_set_focus_color},
    {"hidden_edges", lib_set_hidden_edges},
    {"inner_gaps", lib_set_inner_gaps},
    {"mod", lib_set_mod},
    {"outer_gaps", lib_set_outer_gaps},
    {"repeat_delay", lib_set_repeat_delay},
    {"repeat_rate", lib_set_repeat_rate},
    {"resize_direction", lib_set_resize_direction},
    {"root_color", lib_set_root_color},
    {"sloppy_focus", lib_set_sloppy_focus},
    {"smart_hidden_edges", lib_set_smart_hidden_edges},
    {"workspaces", lib_create_workspaces},
    {NULL, NULL},
};

static const struct luaL_Reg options_getter[] =
{
    /* {"workspaces", lib_create_workspaces}, */
    {"sloppy_focus", lib_get_sloppy_focus},
    /* {"automatic_workspace_naming", lib_lua_idenity_funcion}, */
    /* {"mod", lib_lua_idenity_funcion}, */
    {"inner_gaps", lib_get_inner_gaps},
    /* {"outer_gaps", lib_lua_idenity_funcion}, */
    /* {"default_layout", lib_lua_idenity_funcion}, */
    /* {"border_color", lib_lua_idenity_funcion}, */
    {NULL, NULL},
};

void create_lua_options(struct options *options) {
    if (!options)
        return;
    struct options **user_con = lua_newuserdata(L, sizeof(struct options*));
    *user_con = options;

    luaL_setmetatable(L, CONFIG_OPTIONS);
}

void lua_load_options(lua_State *L)
{
    create_class(L,
            options_meta,
            options_f,
            options_m,
            options_setter,
            options_getter,
            CONFIG_OPTIONS);

    luaL_newlib(L, options_f);
    lua_setglobal(L, "Options");
}

void lua_init_options(struct options *options)
{
    create_lua_options(options);
    lua_setglobal(L, "opt");
}

int lib_reload(lua_State *L)
{
    if (server_is_config_reloading_prohibited()) {
        luaL_where(L, 1);
        const char *where = luaL_checkstring(L, -1);
        char *res = g_strconcat(
                where,
                "action.reload() aborted: infinit recursion detected",
                NULL);
        // from here on using the variable that was on the stack may
        // lead to segfaults because lua may garbage collect them
        lua_pop(L, 1);
        debug_print("line: %s\n", res);
        lua_pushstring(L, res);
        lua_error(L);
    }
    server_prohibit_reloading_config();

    close_error_file();
    init_error_file();

    options_reset(server.default_layout->options);
    server_reset_layout_ring(server.default_layout_ring);

    workspaces_remove_loaded_layouts(server_get_workspaces());
    load_config(L);

    // TODO: do we need to reset all layouts? or is it undesirable?
    for (GList *iterator = server_get_workspaces(); iterator; iterator = iterator->next) {
        struct workspace *ws = iterator->data;
        set_default_layout(ws);
    }

    notify_msg("reloaded config file");

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct workspace *ws = monitor_get_active_workspace(m);
        tagset_focus_tags(ws, ws->prev_workspaces);
    }

    arrange();
    workspace_update_names(server_get_workspaces());
    return 0;
}

int lib_set_arrange_by_focus(lua_State *L)
{
    bool arrange_by_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->arrange_by_focus = arrange_by_focus;
    return 0;
}

int lib_set_automatic_workspace_naming(lua_State *L)
{
    bool automatic_workspace_naming = lua_toboolean(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->automatic_workspace_naming = automatic_workspace_naming;
    ipc_event_workspace();
    return 0;
}

int lib_set_inner_gaps(lua_State *L)
{
    int inner_gap = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->inner_gap = inner_gap;
    return 0;
}

int lib_set_outer_gaps(lua_State *L)
{
    int outer_gap = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->outer_gap = outer_gap;
    return 0;
}

int lib_set_tile_border_width(lua_State *L)
{
    int tile_border_width = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->tile_border_px = tile_border_width;
    return 0;
}

int lib_set_default_layout(lua_State *L)
{
    const char *name = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    server.default_layout->name = strdup(name);
    return 0;
}

int lib_set_float_border_width(lua_State *L)
{
    int float_border_width = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->float_border_px = float_border_width;
    return 0;
}

struct options *check_options(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_OPTIONS);
    luaL_argcheck(L, ud != NULL, 1, "`options' expected");
    return (struct options *)*ud;
}

int lib_set_focus_color(lua_State *L)
{
    struct color color = check_color(L, 2);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->focus_color = color;
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
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    // there are only 4 mods ranging from 1-4
    i = MAX(MIN(i-1, 3), 0);

    options->modkey = i;
    return 0;
}

int lib_set_border_color(lua_State *L)
{
    struct color color = check_color(L, 2);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->border_color = color;
    return 0;
}

int lib_set_root_color(lua_State *L)
{
    struct color color = check_color(L, 2);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->root_color = color;
    return 0;
}

int lib_set_sloppy_focus(lua_State *L)
{
    bool sloppy_focus = lua_toboolean(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->sloppy_focus = sloppy_focus;
    return 0;
}

int lib_set_repeat_rate(lua_State *L)
{
    int repeat_rate = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->repeat_rate = repeat_rate;
    return 0;
}

int lib_set_repeat_delay(lua_State *L)
{
    int repeat_delay = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->repeat_delay = repeat_delay;
    return 0;
}

int lib_set_entry_position_function(lua_State *L)
{
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->new_position_func_ref = lua_func_ref;
    return 0;
}

int lib_set_entry_focus_position_function(lua_State *L)
{
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->new_focus_position_func_ref = lua_func_ref;
    return 0;
}

// TODO refactor this function hard to read
int lib_create_workspaces(lua_State *L)
{
    GPtrArray *tag_names = g_ptr_array_new();

    size_t len = lua_rawlen(L, -1);
    for (int i = 0; i < len; i++) {
        const char *ws_name = get_config_array_str(L, "workspaces", i+1);
        g_ptr_array_add(tag_names, strdup(ws_name));
    }
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    list_clear(options->tag_names, NULL);
    wlr_list_cat(options->tag_names, tag_names);

    load_workspaces(server_get_workspaces(), options->tag_names);

    g_ptr_array_unref(tag_names);

    return 0;
}

int lib_add_rule(lua_State *L)
{
    struct rule *rule = get_config_rule(L);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    GPtrArray *rules = options->rules;
    g_ptr_array_add(rules, rule);
    return 0;
}

int lib_add_mon_rule(lua_State *L)
{
    struct mon_rule *mon_rule = get_config_mon_rule(L);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    GPtrArray *mon_rules = options->mon_rules;
    g_ptr_array_add(mon_rules, mon_rule);
    return 0;
}

int lib_bind_key(lua_State *L)
{
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);

    const char *binding = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    struct keybinding *keybinding = create_keybinding(binding, lua_func_ref);
    options_add_keybinding(options, keybinding);
    return 0;
}

int lib_set_layout_constraints(lua_State *L)
{
    struct resize_constraints layout_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->layout_constraints = layout_constraints;

    return 0;
}

int lib_set_master_constraints(lua_State *L)
{
    struct resize_constraints master_constraints = lua_toresize_constrains(L);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->master_constraints = master_constraints;

    return 0;
}

int lib_set_resize_direction(lua_State *L)
{
    enum wlr_edges resize_dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->resize_dir = resize_dir;

    return 0;
}

int lib_set_hidden_edges(lua_State *L)
{
    enum wlr_edges hidden_edges = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->hidden_edges = hidden_edges;

    return 0;
}

int lib_set_smart_hidden_edges(lua_State *L)
{
    bool smart_hidden_edges = lua_toboolean(L, -1);
    lua_pop(L, 1);

    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    options->smart_hidden_edges = smart_hidden_edges;

    return 0;
}

int lib_get_sloppy_focus(lua_State *L)
{
    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    int sloppy_focus = options->sloppy_focus;
    lua_pushinteger(L, sloppy_focus);
    return 1;
}

int lib_get_inner_gaps(lua_State *L)
{
    struct options *options = check_options(L, 1);
    lua_pop(L, 1);

    int inner_gap = options->inner_gap;
    lua_pushinteger(L, inner_gap);
    return 1;
}
