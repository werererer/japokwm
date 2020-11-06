#include "utils/parseConfigUtils.h"
#include <lua.h>
#include <stdlib.h>
#include <string.h>

void loadConfig(lua_State *L, char *path)
{
    if (luaL_loadfile(L, path)) {
        printf("file didn't load %s\n", luaL_checkstring(L, -1));
        lua_pop(L, 1);
    }
    lua_pcall(L, 0, 0, 0);
}

static const char *getConfigArrayStr(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    const char *s = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    return s;
}

const char *getConfigStr(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    const char *str = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    return str;
}

static float getConfigArrayFloat(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    float f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

float getConfigFloat(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    float f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

static int getConfigArrayInt(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    int f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

int getConfigInt(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return i;
}

bool getConfigBool(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return b;
}

static int getConfigArrayFuncId(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    int r = luaL_ref(L, LUA_REGISTRYINDEX);
    return r;
}

int getConfigFuncId(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    int f = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);
    return f;
}

void callArrangeFunc(lua_State *L, int funcId, int n)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcId);
    lua_pushinteger(L, n);
    lua_pcall(L, 0, 0, 0);
}

static struct layout getConfigArrayLayout(lua_State *L, size_t i)
{
    struct layout layout;
    layout.symbol = (char *)getConfigArrayStr(L, 1);
    layout.funcId = getConfigArrayFuncId(L, 2);
    return layout;
}

static Key getConfigArrayKey(lua_State *L, size_t i)
{
    return getConfigArrayLayout(L, i);
}

struct layout getConfigLayout(lua_State *L, char *name)
{
    struct layout layout;
    lua_getglobal(L, name);
    layout.symbol = (char *)getConfigArrayStr(L, 1);
    layout.funcId = getConfigArrayFuncId(L, 2);
    lua_pop(L, 1);
    return layout;
}

static struct rule getConfigArrayRule(lua_State *L, size_t i)
{
    struct rule rule;
    lua_rawgeti(L, -1, i);
    rule.id  = getConfigArrayStr(L, 1);
    rule.title  = getConfigArrayStr(L, 2);
    rule.tags  = getConfigArrayInt(L, 3);
    rule.floating  = getConfigArrayFloat(L, 4);
    rule.monitor  = getConfigArrayInt(L, 5);
    lua_pop(L, 1);
    return rule;
}


struct rule getConfigRule(lua_State *L, char *name)
{
    struct rule rule;
    rule.id  = getConfigArrayStr(L, 1);
    rule.title  = getConfigArrayStr(L, 2);
    rule.tags  = getConfigArrayInt(L, 3);
    rule.floating  = getConfigArrayFloat(L, 4);
    rule.monitor  = getConfigArrayInt(L, 5);
    return rule;
}

static struct monRule getConfigArrayMonRule(lua_State *L, size_t i)
{
    struct monRule monrule;
    monrule.name = getConfigArrayStr(L, 1);
    monrule.mfact = getConfigArrayFloat(L, 2);
    monrule.nmaster = getConfigArrayInt(L, 3);
    monrule.scale = getConfigArrayFloat(L, 4);
    *monrule.lt = getConfigArrayLayout(L, 5);
    return monrule;
}

struct monRule getConfigMonRule(lua_State *L, char *name)
{
    struct monRule monrule;
    lua_getglobal(L, name);

    monrule.name = getConfigArrayStr(L, 1);
    monrule.mfact = getConfigArrayFloat(L, 2);
    monrule.nmaster = getConfigArrayInt(L, 3);
    monrule.scale = getConfigArrayFloat(L, 4);
    *monrule.lt = getConfigArrayLayout(L, 5);
    return monrule;
}

Key getConfigKey(lua_State *L, char *name)
{
    Key key = (Key)getConfigLayout(L, name);
    return key;
}

void getConfigStrArr(lua_State *L, char **resArr, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        resArr[i] = (char *)getConfigArrayStr(L, i);
}

void getConfigIntArr(lua_State *L, int resArr[], char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        resArr[i] = getConfigArrayInt(L, i);
}

void getConfigFloatArr(lua_State *L, float resArr[], char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 1; i <= len; i++)
        resArr[i] = getConfigArrayFloat(L, i);
}

void getConfigKeyArr(lua_State *L, Key *keys, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        keys[i] = getConfigArrayKey(L, i);
}

void getConfigRuleArr(lua_State *L, struct rule *rules, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        rules[i] = getConfigArrayRule(L, i);
}

void getConfigLayoutArr(lua_State *L, struct layout *layouts, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        layouts[i] = getConfigArrayLayout(L, i);
}

void getConfigMonRuleArr(lua_State *L, struct monRule *monrules, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        monrules[i] = getConfigArrayMonRule(L, i);
}

void callfunc(int funcid)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcid);
    lua_pcall(L, 0, 0, 0);
}
