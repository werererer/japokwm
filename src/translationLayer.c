#include "translationLayer.h"

#include <lauxlib.h>
#include <lua.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_output_layout.h>

#include "cursor.h"
#include "lib/actions/lib_actions.h"
#include "lib/actions/lib_container.h"
#include "lib/config/lib_config.h"
#include "lib/config/local_config.h"
#include "lib/event_handler/lib_event_handler.h"
#include "lib/layout/lib_layout.h"
#include "lib/info/lib_info.h"
#include "lib/event_handler/local_event_handler.h"
#include "lib/monitor/lib_monitor.h"

static const struct luaL_Reg action[] =
{
    {"arrange", lib_arrange},
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
    {"move_container_to_workspace", lib_move_container_to_workspace},
    {"move_resize", lib_move_resize},
    {"move_to_scratchpad", lib_move_to_scratchpad},
    {"quit", lib_quit},
    {"repush", lib_repush},
    {"resize_main", lib_resize_main},
    {"set_floating", lib_set_floating},
    {"set_nmaster", lib_set_nmaster},
    {"show_scratchpad", lib_show_scratchpad},
    {"swap_workspace", lib_swap_workspace},
    {"tag_view", lib_tag_view},
    {"toggle_bars", lib_toggle_bars},
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
    {"set_alpha", container_set_alpha},
    {"set_ratio", container_set_ratio},
    {"set_sticky", container_set_sticky},
    {NULL, NULL},
};

static const struct luaL_Reg event[] =
{
    {"add_listener", lib_add_listener},
    {NULL, NULL},
};

static const struct luaL_Reg localevent[] =
{
    {"add_listener", local_add_listener},
    {NULL, NULL},
};

static const struct luaL_Reg info[] =
{
    {"get_active_layout", lib_get_active_layout},
    {"get_container_under_cursor", lib_get_container_under_cursor},
    {"get_next_empty_workspace", lib_get_next_empty_workspace},
    {"get_nmaster", lib_get_nmaster},
    {"get_previous_layout", lib_get_previous_layout},
    {"get_root_area", lib_get_root_area},
    {"get_this_container_count", lib_get_this_container_count},
    {"get_workspace", lib_get_workspace},
    {"is_container_not_in_limit", lib_is_container_not_in_limit},
    {"is_container_not_in_master_limit", lib_is_container_not_in_master_limit},
    {"stack_position_container_position", lib_stack_position_container_position},
    {"this_container_position", lib_this_container_position},
    {NULL, NULL},
};

static const struct luaL_Reg config[] = 
{
    {"add_mon_rule", lib_add_mon_rule},
    {"add_rule", lib_add_rule},
    {"bind_key", lib_bind_key},
    {"create_layout_set", lib_create_layout_set},
    {"create_workspaces", lib_create_workspaces},
    {"reload", lib_reload},
    {"set_arrange_by_focus", lib_set_arrange_by_focus},
    {"set_automatic_workspace_naming", lib_set_automatic_workspace_naming},
    {"set_border_color", lib_set_border_color},
    {"set_default_layout", lib_set_default_layout},
    {"set_float_borderpx", lib_set_float_borderpx},
    {"set_focus_color", lib_set_focus_color},
    {"set_hidden_edges", lib_set_hidden_edges},
    {"set_inner_gaps", lib_set_inner_gaps},
    {"set_layout_constraints", lib_set_layout_constraints},
    {"set_master_constraints", lib_set_master_constraints},
    {"set_master_layout_data", lib_set_master_layout_data},
    {"set_mod", lib_set_mod},
    {"set_outer_gaps", lib_set_outer_gaps},
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

static const struct luaL_Reg localconfig[] =
{
    {"set_arrange_by_focus", local_set_arrange_by_focus},
    {"set_border_color", local_set_border_color},
    {"set_float_borderpx", local_set_float_borderpx},
    {"set_focus_color", local_set_focus_color},
    {"set_hidden_edges", local_set_hidden_edges},
    {"set_inner_gaps", local_set_inner_gaps},
    {"set_layout_constraints", local_set_layout_constraints},
    {"set_master_constraints", local_set_master_constraints},
    {"set_master_layout_data", local_set_master_layout_data},
    {"set_outer_gaps", local_set_outer_gaps},
    {"set_resize_data", local_set_resize_data},
    {"set_resize_direction", local_set_resize_direction},
    {"set_resize_function", local_set_resize_function},
    {"set_sloppy_focus", local_set_sloppy_focus},
    {"set_smart_hidden_edges", local_set_smart_hidden_edges},
    {"set_tile_borderpx", local_set_tile_borderpx},
    {NULL, NULL},
};

static const struct luaL_Reg layout[] =
{
    {"set", lib_set_layout},
    {NULL, NULL},
};

static const struct luaL_Reg monitor[] = 
{
    {"set_scale", lib_set_scale},
    {"set_transform", lib_set_transform},
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

    lua_pushinteger(L, WLR_DIRECTION_DOWN);
    lua_setfield(L, -2, "bottom");

    lua_pushinteger(L, WLR_DIRECTION_UP);
    lua_setfield(L, -2, "top");

    lua_pushinteger(L, WLR_DIRECTION_LEFT);
    lua_setfield(L, -2, "left");

    lua_pushinteger(L, WLR_DIRECTION_RIGHT);
    lua_setfield(L, -2, "right");

    lua_setfield(L, -2, "direction");

    lua_setglobal(L, "info");
}

void load_lua_api(lua_State *L)
{
    luaL_openlibs(L);

    luaL_newlib(L, action);
    lua_setglobal(L, "action");

    luaL_newlib(L, container);
    lua_setglobal(L, "container");

    luaL_newlib(L, event);
    lua_setglobal(L, "event");

    lua_createtable(L, 0, 0);
    luaL_newlib(L, localevent);
    lua_setfield(L, -2, "event");

    luaL_newlib(L, localconfig);
    lua_setfield(L, -2, "config");
    lua_setglobal(L, "l");

    load_info(L);

    luaL_newlib(L, config);
    lua_setglobal(L, "config");

    luaL_newlib(L, layout);
    lua_setglobal(L, "layout");

    luaL_newlib(L, monitor);
    lua_setglobal(L, "monitor");
}
