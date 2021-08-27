#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include <string.h>
#include "utils/coreUtils.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "layout.h"

/* returns 0 if loading file was successful else return 1
 * the error_file argument gets malloced so it has to be freed */
int load_config(lua_State *L);
int init_utils(lua_State *L);
void init_error_file();
void close_error_file();

// utils
char *get_config_file(const char *file);
/* returned char pointer must be freed */
char *get_config_layout_path();
/* returned char pointer must be freed */
char *get_config_dir(const char *file);
void append_to_lua_path(lua_State *L, const char *path);

// get values
bool get_config_bool(lua_State *L, char *name);
char* get_config_str(lua_State *L, char *name);
float get_config_float(lua_State *L, char *name);
int get_config_int(lua_State *L, char *name);
int lua_call_safe(lua_State *L, int nargs, int nresults, int msgh);
int lua_getglobal_safe(lua_State *L, const char *name);
struct layout get_config_layout(lua_State *L, char *name);
struct monrule get_config_monrule(lua_State *L, char *name);
struct rule get_config_rule(lua_State *L, char *name);
void call_arrange_func(lua_State *L, int funcId, int n);
void notify_msg(const char *msg);
void handle_error(const char *msg);

struct rule get_config_array_rule(lua_State *L, const char* name, size_t i);
char *get_config_array_str(lua_State *L, const char *name, size_t i);
struct monrule get_config_array_monrule(lua_State *L, const char* name, size_t i);

#endif /* PARSE_CONFIG_UTILS_H */
