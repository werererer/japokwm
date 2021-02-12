#include "translationLayer.h"
#include "lib/actions/actions.h"
#include "lib/actions/libcontainer.h"
#include "lib/config/config.h"
#include "lib/config/localconfig.h"
#include "lib/layout/lib_layout.h"
#include "lib/info/info.h"
#include "parseConfig.h"
#include "tile/tile.h"

static const struct luaL_Reg action[] =
{
    {"arrange", lib_arrange},
    {"focus_container", lib_focus_container},
    {"focus_on_hidden_stack", lib_focus_on_hidden_stack},
    {"focus_on_stack", lib_focus_on_stack},
    {"get_nmaster", lib_get_nmaster},
    {"kill", lib_kill},
    {"load_default_layout", lib_load_default_layout},
    {"load_layout", lib_load_layout},
    {"move_container_to_workspace", lib_move_container_to_workspace},
    {"move_resize", lib_move_resize},
    {"quit", lib_quit},
    {"repush", lib_repush},
    {"resize_main", lib_resize_main},
    {"set_floating", lib_set_floating},
    {"set_nmaster", lib_set_nmaster},
    {"increase_default_layout", lib_increase_default_layout},
    {"decrease_default_layout", lib_decrease_default_layout},
    {"increase_nmaster", lib_increase_nmaster},
    {"decrease_nmaster", lib_decrease_nmaster},
    {"spawn", lib_spawn},
    {"toggle_consider_layer_shell", lib_toggle_consider_layer_shell},
    {"toggle_floating", lib_toggle_floating},
    {"toggle_layout", lib_toggle_layout},
    {"toggle_view", lib_toggle_view},
    {"toggle_workspace", lib_toggle_workspace},
    {"view", lib_view},
    {"zoom", lib_zoom},
    {NULL, NULL},
};

static const struct luaL_Reg container[] =
{
    {"container_setsticky", container_setsticky}
};

static const struct luaL_Reg info[] =
{
    {"get_this_container_count", lib_get_this_container_count},
    {"this_container_position", lib_this_container_position},
    {"get_next_empty_workspace", lib_get_next_empty_workspace},
    {"get_workspace", lib_get_workspace},
    {"get_container_under_cursor", lib_get_container_under_cursor},
    {"is_container_not_in_limit", lib_is_container_not_in_limit},
    {"is_container_not_in_master_limit", lib_is_container_not_in_master_limit},
    {NULL, NULL},
};

static const struct luaL_Reg config[] = 
{
    {"reload", lib_reload_config},
    {"set_border_color", lib_set_border_color},
    {"set_tile_borderpx", lib_set_tile_borderpx},
    {"set_float_borderpx", lib_set_float_borderpx},
    {"set_focus_color", lib_set_focus_color},
    {"set_gaps", lib_set_gaps},
    {"set_mod", lib_set_mod},
    {"set_root_color", lib_set_root_color},
    {"set_sloppy_focus", lib_set_sloppy_focus},
    {"set_repeat_rate", lib_set_repeat_rate},
    {"set_repeat_delay", lib_set_repeat_delay},
    {"set_default_layout", lib_set_default_layout},
    {"set_workspaces", lib_set_workspaces},
    {"set_rules", lib_set_rules},
    {"set_layouts", lib_set_layouts},
    {"set_monrules", lib_set_monrules},
    {"set_keybinds", lib_set_keybinds},
    {"set_buttons", lib_set_buttons},
    {"set_layout_constraints", lib_set_layout_constraints},
    {"set_master_constraints", lib_set_master_constraints},
    {"set_update_function", lib_set_update_function},
    {"set_resize_direction", lib_set_resize_direction},
    {"set_master_layout_data", lib_set_master_layout_data},
    {"set_resize_data", lib_set_resize_data},
    {NULL, NULL},
};

static const struct luaL_Reg localconfig[] =
{
    {"set_arrange_by_focus", local_set_arrange_by_focus},
    {"set_border_color", local_set_border_color},
    {"set_tile_borderpx", local_set_tile_borderpx},
    {"set_float_borderpx", local_set_float_borderpx},
    {"set_focus_color", local_set_focus_color},
    {"set_gaps", local_set_gaps},
    {"set_sloppy_focus", local_set_sloppy_focus},
    {"set_layout_constraints", local_set_layout_constraints},
    {"set_master_constraints", local_set_master_constraints},
    {"set_update_function", local_set_update_function},
    {"set_resize_direction", local_set_resize_direction},
    {"set_master_layout_data", local_set_master_layout_data},
    {"set_resize_data", local_set_resize_data},
    {NULL, NULL},
};

static const struct luaL_Reg layout[] =
{
    {"set", lib_set_layout},
    {NULL, NULL},
};

void load_libs(lua_State *L)
{
    luaL_newlib(L, action);
    lua_setglobal(L, "action");
    luaL_newlib(L, container);
    lua_setglobal(L, "container");
    luaL_newlib(L, info);
    lua_setglobal(L, "info");
    luaL_newlib(L, config);
    lua_setglobal(L, "config");
    luaL_newlib(L, localconfig);
    lua_setglobal(L, "lconfig");
    luaL_newlib(L, layout);
    lua_setglobal(L, "layout");
}
