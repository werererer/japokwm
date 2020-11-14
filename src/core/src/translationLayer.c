#include "translationLayer.h"
#include "actions.h"
#include "tile/tile.h"
#include "parseConfig.h"
#include "info.h"

static const struct luaL_Reg action[] =
{
    {"arrangeThis", arrangeThis},
    {"focusOnHiddenStack", focusOnHiddenStack},
    {"focusOnStack", focusOnStack},
    {"kill", killClient},
    {"moveResize", moveResize},
    {"quit", quit},
    {"readOverlay", readOverlay},
    {"spawn", spawn},
    {"tag", tag},
    {"toggleAddView", toggleAddView},
    {"toggleFloating", toggleFloating},
    {"toggleTag", toggletag},
    {"toggleView", toggleView},
    {"updateLayout", updateLayout},
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
