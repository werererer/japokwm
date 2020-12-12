#include "utils/parseConfigUtils.h"
#include "tile/tileTexture.h"
#include "utils/writeFile.h"
#include <lauxlib.h>
#include <wlr/util/log.h>
#include <lua.h>
#include <stdlib.h>
#include <string.h>
#include <translationLayer.h>

/* static write_error(int fd, const char *msg); */

/* static write_error(int fd, const char *msg) */
/* { */
/*     open("/home/jakob/error", ) */
/*     write_to_file(fd, "hi", ); */
/* } */

int load_config(lua_State *L, const char *path, char *error_file)
{
    char *config_file = calloc(1, strlen(path)+strlen("/init.lua"));
    join_path(config_file, path);
    strcpy(error_file, config_file);
    join_path(error_file, "error.msg");
    join_path(config_file, "init.lua");

    if (!path) {
        return 1;
    }
    loadLibs(L);
    if (luaL_loadfile(L, config_file)) {
        wlr_log(WLR_ERROR, "file didn't load %s\n", luaL_checkstring(L, -1));
        lua_pop(L, 1);
        return 1;
    }
    wlr_log(WLR_DEBUG, "load file %s", path);
    lua_pcall(L, 0, LUA_MULTRET, 0);

    return 0;
}

static void handle_error(const char *msg)
{
    printf("ERROR: %s\n", msg);
}


static char *get_config_array_str(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isstring(L, -1)) {
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%lu is not a string", i);
        handle_error(c);
        return "";
    }
    const char *str = luaL_checkstring(L, -1);
    char *termcmd = calloc(strlen(str), sizeof(char));
    strcpy(termcmd, str);
    lua_pop(L, 1);
    return termcmd;
}

char *get_config_str(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    if (!lua_isstring(L, -1)) {
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%s is not a string", name);
        handle_error(c);
        return "";
    }
    const char *str = luaL_checkstring(L, -1);
    char *termcmd = calloc(strlen(str), sizeof(char));
    strcpy(termcmd, str);
    lua_pop(L, 1);
    return termcmd;
}

static float get_config_array_float(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isnumber(L, -1)) {
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%lu is not a number", i);
        handle_error(c);
        return 0;
    }
    float f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

float get_config_float(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    if (!lua_isnumber(L, -1)) {
        /* write_to_file(fd, "ERROR: %s is not a number\n"); */
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%s is not a number", name);
        handle_error(c);
        return 0;
    }
    float f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

static int get_config_array_int(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isinteger(L, -1)) {
        printf("ERROR: %lu is not an integer\n", i);
        return 0;
    }
    int f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

int get_config_int(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    if (!lua_isinteger(L, -1)) {
        printf("ERROR: %s is not an integer\n", name);
        return 0;
    }
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return i;
}

static bool get_config_array_bool(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isboolean(L, -1)) {
        printf("ERROR: %lu is not a boolean\n", i);
        return false;
    }
    bool f = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return f;
}

bool get_config_bool(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    printf("start\n");
    if (!lua_isboolean(L, -1)) {
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%s is not a boolean", name);
        handle_error(c);
        return false;
    }
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return b;
}

static int get_config_array_func_id(lua_State *L, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isfunction(L, -1)) {
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%lu is not a function", i);
        handle_error(c);
        return 0;
    }
    int r = luaL_ref(L, LUA_REGISTRYINDEX);
    return r;
}

int get_config_func_id(lua_State *L, char *name)
{
    lua_getglobal(L, name);
    if (!lua_isfunction(L, -1)) {
        char c[NUM_CHARS] = "";
        snprintf(c, NUM_CHARS, "%s is not a function", name);
        handle_error(c);
        return 0;
    }
    int f = luaL_ref(L, LUA_REGISTRYINDEX);
    return f;
}

void call_arrange_func(lua_State *L, int funcId, int n)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcId);
    lua_pushinteger(L, n);
    lua_pcall(L, 0, 0, 0);
}

void call_function(lua_State *L, struct containers_info cInfo)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, cInfo.id);
    lua_pushinteger(L, cInfo.n);
    lua_pcall(L, 1, 0, 0);
    luaL_ref(L, LUA_REGISTRYINDEX);
}

static struct layout get_config_array_layout(lua_State *L, size_t i)
{
    struct layout layout;
    lua_rawgeti(L, -1, i);
    layout.symbol = get_config_array_str(L, 1);
    layout.funcId = get_config_array_func_id(L, 2);
    lua_pop(L, 1);
    return layout;
}

struct layout get_config_layout(lua_State *L, char *name)
{
    struct layout layout;
    lua_getglobal(L, name);
    layout.symbol = get_config_array_str(L, 1);
    layout.funcId = get_config_array_func_id(L, 2);
    lua_pop(L, 1);
    return layout;
}

static struct rule get_config_array_rule(lua_State *L, size_t i)
{
    struct rule rule;
    lua_rawgeti(L, -1, i);
    rule.id  = get_config_array_str(L, 1);
    rule.title  = get_config_array_str(L, 2);
    rule.tags  = get_config_array_int(L, 3);
    rule.floating  = get_config_array_bool(L, 4);
    rule.monitor  = get_config_array_int(L, 5);
    lua_pop(L, 1);
    return rule;
}


struct rule get_config_rule(lua_State *L, char *name)
{
    struct rule rule;
    rule.id  = get_config_array_str(L, 1);
    rule.title  = get_config_array_str(L, 2);
    rule.tags  = get_config_array_int(L, 3);
    rule.floating  = get_config_array_float(L, 4);
    rule.monitor  = get_config_array_int(L, 5);
    return rule;
}

static struct mon_rule get_config_array_monrule(lua_State *L, size_t i)
{
    struct mon_rule monrule;
    monrule.name = get_config_array_str(L, 1);
    monrule.mfact = get_config_array_float(L, 2);
    monrule.nmaster = get_config_array_int(L, 3);
    monrule.scale = get_config_array_float(L, 4);
    *monrule.lt = get_config_array_layout(L, 5);
    return monrule;
}

struct mon_rule get_config_monrule(lua_State *L, char *name)
{
    struct mon_rule monrule;
    lua_getglobal(L, name);

    monrule.name = get_config_array_str(L, 1);
    monrule.mfact = get_config_array_float(L, 2);
    monrule.nmaster = get_config_array_int(L, 3);
    monrule.scale = get_config_array_float(L, 4);
    *monrule.lt = get_config_array_layout(L, 5);
    return monrule;
}

Key get_config_key(lua_State *L, char *name)
{
    Key key = (Key)get_config_layout(L, name);
    return key;
}

void get_config_str_arr(lua_State *L, struct wlr_list *resArr, char *name)
{
    //TODO cleanup !!!
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 1; i <= len; i++)
        wlr_list_push(resArr, get_config_array_str(L, i));
    lua_pop(L, 1);
}

void get_config_int_arr(lua_State *L, int resArr[], char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        resArr[i] = get_config_array_int(L, i);
}

void get_config_float_arr(lua_State *L, float resArr[], char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 1; i <= len; i++)
        resArr[i-1] = get_config_array_float(L, i);
    lua_pop(L, 1);
}

void get_config_layout_arr(lua_State *L, struct layout *layouts, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    struct layout lt;
    // TODO: cleanup malloc?
    layouts = malloc(sizeof(struct layout)*len);
    for (int i = 1; i <= len; i++) {
        lt = get_config_array_layout(L, 1);
        layouts[i-1].symbol = lt.symbol;
        layouts[i-1].funcId = 4;
    }
    lua_pop(L, 1);
}

void get_config_key_arr(lua_State *L, Key *keys, char *name)
{
    get_config_layout_arr(L, keys, name);
}

void get_config_rule_arr(lua_State *L, struct rule *rules, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 1; i <= len; i++)
        rules[i-1] = get_config_array_rule(L, i);
    lua_pop(L, 1);
}

void get_config_mon_rule_arr(lua_State *L, struct mon_rule *monrules, char *name)
{
    lua_getglobal(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        monrules[i] = get_config_array_monrule(L, i);
}

void call_func(int funcid)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcid);
    lua_pcall(L, 0, 0, 0);
}
