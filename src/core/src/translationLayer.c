#include "translationLayer.h"
#include "actions.h"
#include "tile/tile.h"

static const struct luaL_Reg mylib[] = 
{
    {"focusOnHiddenStack", focusOnHiddenStack},
    {"focusOnStack", focusOnStack},
    {"moveResize", moveResize},
    {"quit", quit},
    {"spawn", spawn},
    {"toggleFloating", toggleFloating},
    {"updateLayout", updateLayout},
    {NULL, NULL},
};

void loadLibs(lua_State *L)
{
    luaL_newlib(L, mylib);
    lua_setglobal(L, "mylib");
}
