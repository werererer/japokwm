#include "lib/lib_cursor_mode.h"

#include "cursor.h"
#include "translationLayer.h"

static const struct luaL_Reg cursor_mode_getter[] =
{
    {"normal", lib_cursor_mode_get_cursor_normal},
    {"move", lib_cursor_mode_get_cursor_move},
    {"resize", lib_cursor_mode_get_cursor_resize},
    {NULL, NULL},
};

void lua_load_cursor_mode(lua_State *L)
{
    create_enum(L, cursor_mode_getter, CONFIG_CURSOR_MODE);

    lua_createtable(L, 0, 0);
    luaL_setmetatable(L, CONFIG_CURSOR_MODE);
    lua_setglobal(L, "Cursor_mode");
}

enum cursor_mode check_cursor_mode(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_CURSOR_MODE);
    luaL_argcheck(L, ud != NULL, narg, "`cursor_mode' expected");
    return *(enum cursor_mode *)ud;
}

static void create_lua_cursor_mode(lua_State *L, enum cursor_mode cursor_mode) {
    enum cursor_mode *user_cursor_mode = lua_newuserdata(L, sizeof(enum cursor_mode));
    *user_cursor_mode = cursor_mode;

    luaL_setmetatable(L, CONFIG_CURSOR_MODE);
}

// getter
int lib_cursor_mode_get_cursor_normal(lua_State *L)
{
    lua_pushinteger(L, CURSOR_NORMAL);
    return 1;
}

int lib_cursor_mode_get_cursor_move(lua_State *L)
{
    lua_pushinteger(L, CURSOR_MOVE);
    return 1;
}

int lib_cursor_mode_get_cursor_resize(lua_State *L)
{
    lua_pushinteger(L, CURSOR_RESIZE);
    return 1;
}
