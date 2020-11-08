#include "translationLayer.h"
#include "actions.h"
#include "tile/tile.h"

static const struct luaL_Reg action[] = 
{
    {"focusOnHiddenStack", focusOnHiddenStack},
    {"focusOnStack", focusOnStack},
    {"moveResize", moveResize},
    {"quit", quit},
    {"spawn", spawn},
    {"toggleFloating", toggleFloating},
    {"updateLayout", updateLayout},
    {"readOverlay", readOverlay},
    {"zoom", zoom},
    {NULL, NULL},
};

void loadLibs(lua_State *L)
{
    luaL_newlib(L, action);
    lua_setglobal(L, "action");
}
