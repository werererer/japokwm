#ifndef PARSE_CONFIG_UTILS_H
#define PARSE_CONFIG_UTILS_H
#include <glib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

GPtrArray *create_default_config_paths();

int load_file(lua_State *L, const char *file);
/* returns 0 if loading file was successful else return 1
 * the error_file argument gets malloced so it has to be freed */
int load_config(lua_State *L);
void load_default_lua_config(lua_State *L);
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
int lua_call_safe(lua_State *L, int nargs, int nresults, int msgh);
int lua_getglobal_safe(lua_State *L, const char *name);
void notify_msg(const char *msg);
void handle_error(const char *msg);

struct rule *get_config_rule(lua_State *L);
const char *get_config_str(lua_State *L, int idx);
const char *get_config_array_str(lua_State *L, const char *name, size_t i);
struct mon_rule *get_config_mon_rule(lua_State *L);

#endif /* PARSE_CONFIG_UTILS_H */
