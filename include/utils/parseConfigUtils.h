#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include <string.h>
#include "utils/coreUtils.h"
#include "monitor.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* returns 0 if loading file was successful
 * else return 1 */
int load_config(lua_State *L, char *path);

Key getConfigKey(lua_State *L, char *name);
struct layout get_config_layout(lua_State *L, char *name);
struct mon_rule getConfigMonRule(lua_State *L, char *name);
struct rule getConfigRule(lua_State *L, char *name);
char* get_config_str(lua_State *L, char *name);
float getConfigFloat(lua_State *L, char *name);
int get_config_int(lua_State *L, char *name);
bool get_config_bool(lua_State *L, char *name);
int getConfigFuncId(lua_State *L, char *name);
void callArrangeFunc(lua_State *L, int funcId, int n);
void callFunction(lua_State *L, struct containers_info cInfo);

// array
void get_config_str_arr(lua_State *L, struct wlr_list *resArr, char *name);
void get_config_float_arr(lua_State *L, float *resArr, char *name);
void getConfigIntArr(lua_State *L, int *resArr, char *name);
void getConfigLayoutArr(lua_State *L, struct layout *layouts, char *name);
void get_config_key_arr(lua_State *L, Key *keys, char *name);
void get_config_rule_arr(lua_State *L, struct rule *rules, char *name);
void getConfigMonRuleArr(lua_State *L, struct mon_rule *monrules, char *name);

//utils
void callfunc(int funcid);

#endif /* PARSE_CONFIG_UTILS_H */
