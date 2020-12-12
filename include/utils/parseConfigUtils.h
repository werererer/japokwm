#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include <string.h>
#include "utils/coreUtils.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/* returns 0 if loading file was successful else return 1
 * the error_file argument gets malloced so it has to be freed */
int load_config(lua_State *L, const char *path, char *error_file);

Key get_config_key(lua_State *L, char *name);
struct layout get_config_layout(lua_State *L, char *name);
struct mon_rule get_config_monrule(lua_State *L, char *name);
struct rule get_config_rule(lua_State *L, char *name);
char* get_config_str(lua_State *L, char *name);
float get_config_float(lua_State *L, char *name);
int get_config_int(lua_State *L, char *name);
bool get_config_bool(lua_State *L, char *name);
int get_config_func_id(lua_State *L, char *name);
void call_arrange_func(lua_State *L, int funcId, int n);
void call_function(lua_State *L, struct containers_info cInfo);

// array
void get_config_str_arr(lua_State *L, struct wlr_list *resArr, char *name);
void get_config_float_arr(lua_State *L, float *resArr, char *name);
void get_config_int_arr(lua_State *L, int *resArr, char *name);
void get_config_layout_arr(lua_State *L, struct layout *layouts, char *name);
void get_config_key_arr(lua_State *L, Key *keys, char *name);
void get_config_rule_arr(lua_State *L, struct rule *rules, char *name);
void get_config_mon_rule_arr(lua_State *L, struct mon_rule *monrules, char *name);

//utils
void call_func(int funcid);

#endif /* PARSE_CONFIG_UTILS_H */
