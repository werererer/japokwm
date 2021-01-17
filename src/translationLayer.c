#include "translationLayer.h"
#include "lib/actions/actions.h"
#include "lib/actions/libcontainer.h"
#include "lib/info/info.h"
#include "parseConfig.h"
#include "tile/tile.h"
#include "lib/actions/overlay.h"

static const struct luaL_Reg action[] =
{
    {"arrange_this", arrange_this},
    {"set_tabcount", set_tabcount},
    {"set_nmaster", set_nmaster},
    {"set_layout", set_layout},
    {"get_nmaster", get_nmaster},
    {"get_tabcount", get_tabcount},
    /* {"create_overlay", create_overlay}, */
    {"focus_on_hidden_stack", focus_on_hidden_stack},
    {"toggle_consider_layer_shell", toggle_consider_layer_shell},
    {"focus_on_stack", focus_on_stack},
    {"get_overlay", get_overlay},
    {"kill", kill_client},
    {"move_resize", move_resize},
    {"move_client_to_workspace", move_client_to_workspace},
    {"quit", quit},
    {"load_layout", load_layout_lib},
    {"set_overlay", set_overlay},
    {"spawn", spawn},
    {"tag", tag},
    {"set_floating", set_floating},
    {"toggle_floating", toggle_floating},
    {"toggle_tag", toggle_tag},
    {"toggle_view", toggle_view},
    {"update_layout", update_layout},
    {"view", view},
    {"write_this_overlay", write_this_overlay},
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
}
