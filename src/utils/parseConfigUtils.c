#include "utils/parseConfigUtils.h"
#include <wlr/util/log.h>
#include <lua.h>
#include <stdlib.h>
#include <string.h>
#include <translationLayer.h>

int loadConfig(lua_State *L, char *path)
{
    if (!path) {
        return 1;
    }
    loadLibs(L);
    if (luaL_loadfile(L, path)) {
        wlr_log(WLR_ERROR, "file didn't load %s\n", luaL_checkstring(L, -1));
        lua_pop(L, 1);
        return 1;
    }
    wlr_log(WLR_DEBUG, "load file %s", path);
    lua_pcall(L, 0, LUA_MULTRET, 0);
    return 0;
}

static char *getConfigArrayStr(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    const char *str = luaL_checkstring(L, -1);
    char *termcmd = calloc(strlen(str), sizeof(char));
    strcpy(termcmd, str);
    lua_pop(L, 1);
    return termcmd;
}

char *getConfigStr(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    const char *str = luaL_checkstring(L, -1);
    char *termcmd = calloc(strlen(str), sizeof(char));
    strcpy(termcmd, str);
    lua_pop(L, 1);
    return termcmd;
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

static bool getConfigArrayBool(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    int f = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return f;
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
    return f;
}

void callArrangeFunc(lua_State *L, int funcId, int n)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcId);
    lua_pushinteger(L, n);
    lua_pcall(L, 0, 0, 0);
}

void callFunction(lua_State *L, struct containersInfo cInfo)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, cInfo.id);
    lua_pushinteger(L, cInfo.n);
    lua_pcall(L, 1, 0, 0);
    luaL_ref(L, LUA_REGISTRYINDEX);
}

static struct layout getConfigArrayLayout(lua_State *L, size_t i)
{
    struct layout layout;
    lua_rawgeti(L, -1, i);
    layout.symbol = getConfigArrayStr(L, 1);
    layout.funcId = getConfigArrayFuncId(L, 2);
    lua_pop(L, 1);
    return layout;
}

struct layout getConfigLayout(lua_State *L, char *name)
{
    struct layout layout;
    lua_getglobal(L, name);
    layout.symbol = getConfigArrayStr(L, 1);
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
    rule.floating  = getConfigArrayBool(L, 4);
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

void getConfigStrArr(lua_State *L, struct wlr_list *resArr, char *name)
{
    //TODO cleanup !!!
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 1; i <= len; i++)
        wlr_list_push(resArr, getConfigArrayStr(L, i));
    lua_pop(L, 1);
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
        resArr[i-1] = getConfigArrayFloat(L, i);
    lua_pop(L, 1);
}

void getConfigLayoutArr(lua_State *L, struct layout *layouts, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    struct layout lt;
    // TODO: cleanup malloc?
    layouts = malloc(sizeof(*layouts)*len);
    for (int i = 1; i <= len; i++) {
        lt = getConfigArrayLayout(L, 1);
        layouts[i-1].symbol = lt.symbol;
        layouts[i-1].funcId = 4;
    }
    lua_pop(L, 1);
}

void getConfigKeyArr(lua_State *L, Key *keys, char *name)
{
    getConfigLayoutArr(L, keys, name);
}

void getConfigRuleArr(lua_State *L, struct rule *rules, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 1; i <= len; i++)
        rules[i-1] = getConfigArrayRule(L, i);
    lua_pop(L, 1);
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
