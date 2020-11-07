#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include <string.h>
#include "utils/coreUtils.h"
#include "monitor.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void loadConfig(lua_State *L, char *path);
Key getConfigKey(lua_State *L, char *name);
struct layout getConfigLayout(lua_State *L, char *name);
struct monRule getConfigMonRule(lua_State *L, char *name);
struct rule getConfigRule(lua_State *L, char *name);
char* getConfigStr(lua_State *L, char *name);
float getConfigFloat(lua_State *L, char *name);
int getConfigInt(lua_State *L, char *name);
bool getConfigBool(lua_State *L, char *name);
int getConfigFuncId(lua_State *L, char *name);
void callArrangeFunc(lua_State *L, int funcId, int n);
void callFunction(lua_State *L, struct containersInfo cInfo);

// array
void getConfigStrArr(lua_State *L, char **resArr, char *name);
void getConfigFloatArr(lua_State *L, float *resArr, char *name);
void getConfigIntArr(lua_State *L, int *resArr, char *name);
void getConfigLayoutArr(lua_State *L, struct layout *layouts, char *name);
void getConfigKeyArr(lua_State *L, Key *keys, char *name);
void getConfigRuleArr(lua_State *L, struct rule *rules, char *name);
void getConfigMonRuleArr(lua_State *L, struct monRule *monrules, char *name);

//utils
void callfunc(int funcid);

#endif /* PARSE_CONFIG_UTILS_H */
