#include "translationLayer.h"
#include "lib/actions/actions.h"
#include "lib/info/info.h"
#include "parseConfig.h"
#include "tile/tile.h"

static const struct luaL_Reg action[] =
{
    {"arrangeThis", arrange_this},
    {"focusOnHiddenStack", focus_on_hidden_stack},
    {"focusOnStack", focus_on_stack},
    {"kill", killClient},
    {"moveResize", moveResize},
    {"quit", quit},
    {"createOverlay", },
    {"readOverlay", readOverlay},
    {"spawn", spawn},
    {"tag", tag},
    {"toggleAddView", toggleAddView},
    {"toggleFloating", toggleFloating},
    {"toggleTag", toggletag},
    {"toggleView", toggleView},
    {"updateLayout", update_layout},
    {"view", view},
    {"zoom", zoom},
    {NULL, NULL},
};

static const struct luaL_Reg info[] =
{
    {"thisTiledClientCount", thisTiledClientCount},
    {"thisClientPos", thisClientPos},
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
