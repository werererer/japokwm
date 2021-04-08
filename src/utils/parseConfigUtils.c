#include "utils/parseConfigUtils.h"
#include "options.h"
#include "server.h"
#include "utils/writeFile.h"
#include <lauxlib.h>
#include <wlr/util/log.h>
#include <lua.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <translationLayer.h>
#include <execinfo.h>
#include "stringop.h"
#include "utils/coreUtils.h"

static const char *config_paths[] = {
    "$HOME/.config/japokwm/",
    "$XDG_CONFIG_HOME/japokwm/",
    "/etc/japokwm/",
};

static const char *config_file = "init.lua";
static const char *error_file = "init.err";
static int error_fd = -1;

static int load_file(lua_State *L, const char *path, const char *file);

// returns 0 upon success and 1 upon failure
static int load_file(lua_State *L, const char *path, const char *file)
{
    char config_file[strlen(path)+strlen(file)];
    strcpy(config_file, "");
    join_path(config_file, path);
    join_path(config_file, file);

    if (!file_exists(config_file)) {
        return 1;
    }

    if (luaL_loadfile(L, config_file)) {
        const char *errmsg = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        handle_error(errmsg);
        return 1;
    }

    int ret = lua_call_safe(L, 0, 0, 0);

    return ret;
}

char *get_config_file(const char *file)
{
    for (size_t i = 0; i < LENGTH(config_paths); ++i) {
        char *path = strdup(config_paths[i]);
        expand_path(&path);
        path = realloc(path, strlen(path) + strlen(file));
        join_path(path, file);
        if (file_exists(path))
            return path;
        free(path);
    }
    return NULL;
}

char *get_config_layout_path()
{
    return get_config_file("layouts");
}

char *get_config_dir(const char *file)
{
    if (strcmp(server.config_dir, "") != 0 && dir_exists(server.config_dir)) {
        return strdup(server.config_dir);
    }

    char *abs_file = get_config_file(file);
    return dirname(abs_file);
}

void append_to_lua_path(lua_State *L, const char *path)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    const char * curr_path = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    char path_var[strlen(curr_path) + 1 + strlen(path) + strlen("/?.lua")];
    strcpy(path_var, curr_path);
    strcat(path_var, ";");
    strcat(path_var, path);
    join_path(path_var, "/?.lua");

    lua_pushstring(L, path_var);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
}

// returns 0 upon success and 1 upon failure
static int load_default_config(lua_State *L)
{
    char *config_path = get_config_dir(config_file);
    printf("config_dir: %s\n", config_path);

    // get the index of the config file in config_paths array
    int default_id = -1;
    for (int i = 0; i < LENGTH(config_paths); i++) {
        char *path = strdup(config_paths[i]);
        expand_path(&path);
        if (path_compare(path, config_path) == 0) {
            default_id = i;
            free(path);
            break;
        }
        free(path);
    }

    bool loaded_custom_path = false;
    if (default_id == -1) {
        default_id = 0;
        // try to load the file given by config_path
        char *path = strdup(config_path);
        expand_path(&path);

        append_to_lua_path(L, config_path);

        loaded_custom_path = (load_file(L, path, config_file) == EXIT_SUCCESS);
        free(path);
    }

    if (config_path)
        free(config_path);

    if (loaded_custom_path)
        return EXIT_SUCCESS;

    int success = EXIT_SUCCESS;
    // repeat loop until the first config file was loaded successfully
    for (int i = 0; i < LENGTH(config_paths); i++) {
        if (i < default_id)
            continue;

        char *path = strdup(config_paths[i]);
        expand_path(&path);

        append_to_lua_path(L, config_paths[i]);

        if (load_file(L, path, config_file) == EXIT_FAILURE) {
            free(path);
            continue;
        }

        // when config loaded successfully break;
        success = 0;
        free(path);
        break;
    }
    return success;
}

// returns 0 upon success and 1 upon failure
int load_config(lua_State *L)
{
    int success = 0;
    if (strcmp(server.config_file, "") != 0) {
        printf("load custom config file: %s\n", server.config_file);
        success = load_file(L, "", server.config_file);
    } else {
        success = load_default_config(L);
    }
    return success;
}

// returns 0 upon success and 1 upon failure
int init_utils(lua_State *L)
{
    const char *tile_file = "tile.lua";
    char *config_dir = get_config_dir(tile_file);

    if (!config_dir)
        return 1;

    // get the value of the
    int default_id = 0;
    for (int i = 0; i < LENGTH(config_paths); i++) {
        char *path = strdup(config_paths[default_id]);
        expand_path(&path);
        if (path_compare(path, config_dir) == 0) {
            default_id = i;
            break;
        }
        free(path);
    }
    free(config_dir);

    bool success = true;
    // repeat loop until the first config file was loaded successfully
    for (int i = 0; i < LENGTH(config_paths); i++) {
        if (i < default_id)
            continue;

        char *path = strdup(config_paths[i]);
        expand_path(&path);

        append_to_lua_path(L, config_paths[i]);

        if (load_file(L, path, tile_file))
            continue;

        // when config loaded successfully break;
        success = false;
        break;
    }
    return success;
}

void init_error_file()
{
    char *ef = get_config_file("");
    ef = realloc(ef, strlen(ef)+strlen(error_file));
    join_path(ef, error_file);
    error_fd = open(ef, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(ef);
}

void close_error_file()
{
    close(error_fd);
    error_fd = -1;
}

int lua_call_safe(lua_State *L, int nargs, int nresults, int msgh)
{
    int lua_status = lua_pcall(L, nargs, nresults, msgh);
    if (lua_status != LUA_OK) {
        const char *errmsg = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        handle_error(errmsg);
    }
    return lua_status;
}

int lua_getglobal_safe(lua_State *L, const char *name)
{
    lua_getglobal(L, name);
    if (lua_isnil(L, -1))
    {
        char c[NUM_CHARS] = "";
        handle_error(c);
        lua_pop(L, 1);
        return LUA_ERRRUN;
    }
    return LUA_OK;
}

void handle_error(const char *msg)
{
    wlr_log(WLR_ERROR, "%s", msg);

    // if error file not initialized
    if (error_fd < 0)
        return;

    write_to_file(error_fd, msg);
    write_to_file(error_fd, "\n");
}

char *get_config_array_str(lua_State *L, const char *name, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isstring(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return NULL;
    }
    const char *str = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    char *termcmd = strdup(str);
    return termcmd;
}

char *get_config_str(lua_State *L, char *name)
{
    lua_getglobal_safe(L, name);
    if (!lua_isstring(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return "";
    }
    const char *str = luaL_checkstring(L, -1);
    char *termcmd = calloc(strlen(str), sizeof(char));
    strcpy(termcmd, str);
    lua_pop(L, 1);
    return termcmd;
}

static float get_config_array_float(lua_State *L, const char *name, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isnumber(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return 0;
    }
    float f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

float get_config_float(lua_State *L, char *name)
{
    lua_getglobal_safe(L, name);
    if (!lua_isnumber(L, -1)) {
        /* write_to_file(fd, "ERROR: %s is not a number\n"); */
        char c[NUM_CHARS] = "";
        handle_error(c);
        return 0;
    }
    float f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

static int get_config_array_int(lua_State *L, const char *name, size_t i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isinteger(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return 0;
    }
    int f = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    return f;
}

int get_config_int(lua_State *L, char *name)
{
    lua_getglobal_safe(L, name);
    if (!lua_isinteger(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return 0;
    }
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return i;
}

/* static bool get_config_array_bool(lua_State *L, const char *name, size_t i) */
/* { */
/*     lua_rawgeti(L, -1, i); */
/*     if (!lua_isboolean(L, -1)) { */
/*         return false; */
/*     } */
/*     bool f = lua_toboolean(L, -1); */
/*     lua_pop(L, 1); */
/*     return f; */
/* } */

bool get_config_bool(lua_State *L, char *name)
{
    lua_getglobal_safe(L, name);
    if (!lua_isboolean(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return false;
    }
    bool b = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return b;
}

static int get_config_array_func_id(lua_State *L, const char *name, int i)
{
    lua_rawgeti(L, -1, i);
    if (!lua_isfunction(L, -1)) {
        char c[NUM_CHARS] = "";
        handle_error(c);
        return 0;
    }
    int f = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_pop(L, 1);

    return f;
}

void call_arrange_func(lua_State *L, int funcId, int n)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcId);
    lua_pushinteger(L, n);
    lua_call_safe(L, 1, 0, 0);
}

void call_function(lua_State *L, struct layout lt)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, lt.lua_layout_ref);
    lua_pushinteger(L, lt.n_area);
    lua_call_safe(L, 1, 0, 0);
}

struct layout get_config_layout(lua_State *L, char *name)
{
    lua_getglobal_safe(L, name);
    struct layout layout = {
        .symbol = get_config_array_str(L, name, 1),
        .name = get_config_array_str(L, name, 2),
        .n_area = 1,
        .nmaster = 1,
        .lua_layout_ref = 0,
        .lua_layout_copy_data_ref = 0,
        .lua_layout_original_copy_data_ref = 0,
        .options = get_default_options(),
    };
    lua_pop(L, 1);
    return layout;
}

struct rule get_config_array_rule(lua_State *L, const char* name, size_t i)
{
    struct rule rule;
    lua_rawgeti(L, -1, i);

    rule.id  = get_config_array_str(L, name, 1);
    rule.title  = get_config_array_str(L, name, 2);
    rule.lua_func_ref = get_config_array_func_id(L, name, 3);

    lua_pop(L, 1);
    return rule;
}

struct rule get_config_rule(lua_State *L, char *name)
{
    struct rule rule;
    rule.id  = get_config_array_str(L, name, 1);
    rule.title  = get_config_array_str(L, name, 2);
    rule.lua_func_ref = get_config_array_func_id(L, name, 3);
    return rule;
}

struct monrule get_config_array_monrule(lua_State *L, const char* name, size_t i)
{
    struct monrule monrule;
    lua_rawgeti(L, -1, i);

    monrule.name = get_config_array_str(L, name, 1);
    monrule.lua_func_ref = get_config_array_func_id(L, name, 2);
    return monrule;
}

struct monrule get_config_monrule(lua_State *L, char *name)
{
    struct monrule monrule;
    lua_getglobal_safe(L, name);

    monrule.name = get_config_array_str(L, name, 1);
    monrule.lua_func_ref = get_config_array_func_id(L, name, 2);

    lua_pop(L, 1);
    return monrule;
}

struct layout get_config_key(lua_State *L, char *name)
{
    struct layout key = (struct layout)get_config_layout(L, name);
    return key;
}

void get_config_str_arr(lua_State *L, struct wlr_list *resArr, char *name)
{
    lua_getglobal_safe(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        wlr_list_push(resArr, get_config_array_str(L, name, i+1));
    lua_pop(L, 1);
}

void get_config_int_arr(lua_State *L, int resArr[], char *name)
{
    lua_getglobal_safe(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        resArr[i] = get_config_array_int(L, name, i);
}

void get_config_float_arr(lua_State *L, float resArr[], char *name)
{
    lua_getglobal_safe(L, name);
    size_t len = lua_rawlen(L, -1);

    for (int i = 0; i < len; i++)
        resArr[i] = get_config_array_float(L, name, i+1);
    lua_pop(L, 1);
}

void get_config_rule_arr(lua_State *L, struct rule **rules, size_t *rule_count, char *name)
{
    lua_getglobal_safe(L, name);
    size_t len = lua_rawlen(L, -1);
    *rule_count = len;
    *rules = calloc(len, sizeof(struct rule));

    for (int i = 0; i < len; i++)
        *rules[i] = get_config_array_rule(L, name, i+1);

    lua_pop(L, 1);
}

void get_config_mon_rule_arr(lua_State *L, struct monrule **monrules, size_t *monrule_count, char *name)
{
    lua_getglobal_safe(L, name);
    size_t len = lua_rawlen(L, -1);
    *monrule_count = len;
    *monrules = calloc(len, sizeof(struct monrule));

    for (int i = 0; i < len; i++) {
        *monrules[i-1] = get_config_array_monrule(L, name, i+1);
    }

    lua_pop(L, 1);
}

void call_func(int funcid)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcid);
    lua_call_safe(L, 0, 0, 0);
}
