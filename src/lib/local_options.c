#include "lib/local_options.h"

#include "server.h"
#include "lib/lib_options.h"
#include "translationLayer.h"

static const struct luaL_Reg local_options_f[] = 
{
    {"reload", lib_reload},
    {NULL, NULL},
};

static const struct luaL_Reg local_options_m[] =
{
    {"add_mon_rule", lib_add_mon_rule},
    {"add_rule", lib_add_rule},
    {"bind_key", lib_bind_key},
    {"create_layout_set", lib_create_layout_set},
    {"set_layout_constraints", lib_set_layout_constraints},
    {"set_master_constraints", lib_set_master_constraints},
    {"set_master_layout_data", lib_set_master_layout_data},
    {"set_mod", lib_set_mod},
    {"set_repeat_delay", lib_set_repeat_delay},
    {"set_repeat_rate", lib_set_repeat_rate},
    {"set_resize_data", lib_set_resize_data},
    {"set_resize_direction", lib_set_resize_direction},
    {"set_resize_function", lib_set_resize_function},
    {"set_root_color", lib_set_root_color},
    {"set_sloppy_focus", lib_set_sloppy_focus},
    {"set_smart_hidden_edges", lib_set_smart_hidden_edges},
    {"set_tile_borderpx", lib_set_tile_borderpx},
    {NULL, NULL},
};


static const struct luaL_Reg local_options_setter[] =
{
    {"workspaces", lib_create_workspaces},
    {"sloppy_focus", lib_set_sloppy_focus},
    {"automatic_workspace_naming", lib_set_automatic_workspace_naming},
    {"mod", lib_set_mod},
    {"inner_gaps", lib_set_inner_gaps},
    {"outer_gaps", lib_set_outer_gaps},
    {"default_layout", lib_set_default_layout},
    {"border_color", lib_set_border_color},

    {"arrange_by_focus", lib_set_arrange_by_focus},
    {"entry_focus_position_function", lib_set_entry_focus_position_function},
    {"entry_position_function", lib_set_entry_position_function},
    {"float_borderpx", lib_set_float_borderpx},
    {"focus_color", lib_set_focus_color},
    {"hidden_edges", lib_set_hidden_edges},
    {NULL, NULL},
};

static const struct luaL_Reg local_options_getter[] =
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

void lua_load_local_options(struct options *options)
{
    create_class(
            local_options_f,
            local_options_m,
            local_options_setter,
            local_options_getter,
            CONFIG_LOCAL_OPTIONS);

    create_lua_options(server.default_layout->options);
    lua_setglobal(L, "opt");

    luaL_newlib(L, local_options_f);
    lua_setglobal(L, "Options");
}

