#include "translationLayer.h"
#include "lib/actions/actions.h"
#include "lib/info/info.h"
#include "parseConfig.h"
#include "tile/tile.h"
#include "lib/actions/overlay.h"

static const struct luaL_Reg action[] =
{
    {"arrangeThis", arrange_this},
    {"setTabcount", set_tabcount},
    {"getTabcount", get_tabcount},
    /* {"createOverlay", create_overlay}, */
    {"focusOnHiddenStack", focus_on_hidden_stack},
    {"toggleConsiderLayerShell", toggle_consider_layer_shell},
    {"focusOnStack", focus_on_stack},
    {"getOverlay", get_overlay},
    {"kill", kill_client},
    {"moveResize", move_resize},
    {"moveResize", move_resize},
    {"quit", quit},
    {"readLayout", read_layout},
    {"setOverlay", set_overlay},
    {"spawn", spawn},
    {"tag", tag},
    {"setFloating", set_floating},
    {"toggleAddView", toggle_add_view},
    {"toggleFloating", toggle_floating},
    {"toggleTag", toggle_tag},
    {"toggleView", toggle_view},
    {"updateLayout", update_layout},
    {"view", view},
    {"writeThisOverlay", write_this_overlay},
    {"zoom", zoom},
    {NULL, NULL},
};

static const struct luaL_Reg info[] =
{
    {"thisTiledClientCount", this_tiled_client_count},
    {"thisContainerPosition", this_container_position},
    {NULL, NULL},
};

static const struct luaL_Reg config[] = 
{
    {"reload", reloadConfig},
    {NULL, NULL},
};

void loadLibs(lua_State *L)
{
    luaL_newlib(L, action);
    lua_setglobal(L, "action");
    luaL_newlib(L, info);
    lua_setglobal(L, "info");
    luaL_newlib(L, config);
    lua_setglobal(L, "config");
}
