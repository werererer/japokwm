#include "translationLayer.h"
#include "lib/actions/actions.h"
#include "lib/actions/libcontainer.h"
#include "lib/config/config.h"
#include "lib/config/localconfig.h"
#include "lib/info/info.h"
#include "parseConfig.h"
#include "tile/tile.h"

static const struct luaL_Reg action[] =
{
    {"arrange_this", arrange_this},
    {"set_tabcount", set_tabcount},
    {"set_nmaster", set_nmaster},
    {"set_layout", set_layout},
    {"set_arrange_by_focus", set_arrange_by_focus},
    {"set_resize_direction", set_resize_direction},
    {"resize_main", resize_main},
    {"get_nmaster", get_nmaster},
    {"get_tabcount", get_tabcount},
    /* {"create_overlay", create_overlay}, */
    {"focus_on_hidden_stack", focus_on_hidden_stack},
    {"toggle_consider_layer_shell", toggle_consider_layer_shell},
    {"focus_on_stack", focus_on_stack},
    {"kill", kill_client},
    {"move_resize", move_resize},
    {"move_client_to_workspace", move_client_to_workspace},
    {"quit", quit},
    {"load_layout", load_layout_lib},
    {"spawn", spawn},
    {"tag", tag},
    {"set_floating", set_floating},
    {"toggle_floating", toggle_floating},
    {"toggle_tag", toggle_tag},
    {"toggle_view", toggle_view},
    {"view", view},
    {"zoom", zoom},
    {NULL, NULL},
};

static const struct luaL_Reg container[] =
{
    {"container_setsticky", container_setsticky}
};

static const struct luaL_Reg info[] =
{
    {"get_this_container_count", get_this_container_count},
    {"this_container_position", this_container_position},
    {NULL, NULL},
};

static const struct luaL_Reg config[] = 
{
    {"reload", reload_config},
    {"set_gaps", lib_set_gaps},
    {"set_borderpx", lib_set_borderpx},
    {"set_sloppy_focus", lib_set_sloppy_focus},
    {"set_root_color", lib_set_root_color},
    {"set_focus_color", lib_set_focus_color},
    {"set_border_color", lib_set_border_color},
    {NULL, NULL},
};

static const struct luaL_Reg localconfig[] =
{
    {"set_borderpx", local_set_borderpx},
    {"set_gaps", local_set_gaps},
    {"set_sloppy_focus", local_set_sloppy_focus},
    {"set_focus_color", local_set_focus_color},
    {"set_border_color", local_set_border_color},
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
}
