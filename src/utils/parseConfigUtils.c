#include "utils/parseConfigUtils.h"

#include <lauxlib.h>
#include <wlr/util/log.h>
#include <lua.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <translationLayer.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <libnotify/notify.h>
#include <pthread.h>
#include <fts.h>

#include "options.h"
#include "server.h"
#include "utils/writeFile.h"
#include "stringop.h"
#include "utils/coreUtils.h"

static const char *config_paths[] = {
    "$HOME/.config/japokwm/",
    "$XDG_CONFIG_HOME/japokwm/",
    "/etc/japokwm/",
};

static const char *plugin_relative_paths[] = {
    "autoload",
    "plugins",
};

static const char *config_file = "init.lua";
static const char *error_file = "init.err";
static int error_fd = -1;

static int load_file(lua_State *L, const char *file);

// returns 0 upon success and 1 upon failure
static int load_file(lua_State *L, const char *file)
{
    printf("load file: %s\n", file);
    if (!file_exists(file)) {
        return EXIT_FAILURE;
    }

    if (luaL_loadfile(L, file)) {
        const char *errmsg = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        handle_error(errmsg);
        return EXIT_FAILURE;
    }

    int ret = lua_call_safe(L, 0, 0, 0);

    return ret;
}

char *get_config_file(const char *file)
{
    for (size_t i = 0; i < LENGTH(config_paths); ++i) {
        char *path = strdup(config_paths[i]);
        expand_path(&path);
        join_path(&path, file);
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
    if (!abs_file)
        return NULL;

    return dirname(abs_file);
}

void append_to_lua_path(lua_State *L, const char *path)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    const char *curr_path = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    char *path_var = malloc(strlen(curr_path) + strlen(path) + 2);
    strcpy(path_var, curr_path);
    strcat(path_var, ";");
    strcat(path_var, path);

    lua_pushstring(L, path_var);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
    free(path_var);
}

static int load_default_plugins(lua_State *L)
{
    return 1;
}

static int compare(const FTSENT **one, const FTSENT **two)
{
    return strcmp((*one)->fts_name, (*two)->fts_name);
}

static int load_plugins(lua_State *L)
{
    for (int i = 0; i < LENGTH(plugin_relative_paths); i++) {
        const char *path = plugin_relative_paths[i];
        char *base_path = get_config_dir("init.lua");
        join_path(&base_path, path);
        join_path(&base_path, "?");
        join_path(&base_path, "init.lua");
        printf("base_path: %s\n", base_path);
        append_to_lua_path(L, base_path);
    }
    return 1;
}

static int get_base_dir_id()
{
    char *config_path = get_config_dir(config_file);

    // get the index of the config file in config_paths array
    int default_id = -1;
    for (int i = 0; i < LENGTH(config_paths); i++) {
        char *path = strdup(config_paths[i]);
        expand_path(&path);
        if (path_compare(path, config_path) == 0) {
            default_id = i;
            free(path);
            return default_id;
        }
        free(path);
    }
    return default_id;
}

// returns 0 upon success and 1 upon failure
static int load_default_config(lua_State *L)
{
    char *file_path = get_config_file(config_file);
    if (!file_path)
        return EXIT_FAILURE;

    int success = load_file(L, file_path);
    free(file_path);

    return success;
}

// returns 0 upon success and 1 upon failure
int load_config(lua_State *L)
{
    load_plugins(L);
    int success = 0;
    if (strcmp(server.config_file, "") != 0) {
        printf("load custom config file: %s\n", server.config_file);
        success = load_file(L, server.config_file);
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
        return EXIT_FAILURE;

    char *dir = strdup(config_dir);
    join_path(&dir, "?.lua");
    append_to_lua_path(L, dir);
    free(dir);

    char *file_path = strdup(config_dir);
    join_path(&file_path, tile_file);
    int success = load_file(L, file_path);

    free(config_dir);
    free(file_path);

    return success;
}

void init_error_file()
{
    char *ef = get_config_file("");
    join_path(&ef, error_file);
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

static void *_notify_msg(void *arg)
{
    char *msg = arg;
    notify_init(msg);
    NotifyNotification* n = notify_notification_new ("Error in config file", 
            msg,
            0);
    notify_notification_set_timeout(n, 10000); // 10 seconds

    if (!notify_notification_show(n, 0)) 
    {
        printf("show has failed!\n");
    }
    return NULL;
}

void notify_msg(const char *msg)
{
    pthread_t thread;
    pthread_create(&thread, NULL, _notify_msg, (char *)msg);
    pthread_detach(thread);
}

void handle_error(const char *msg)
{
    notify_msg(msg);
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

struct monrule get_config_array_monrule(lua_State *L, const char* name, size_t i)
{
    struct monrule monrule;
    lua_rawgeti(L, -1, i);

    monrule.name = get_config_array_str(L, name, 1);
    monrule.lua_func_ref = get_config_array_func_id(L, name, 2);
    return monrule;
}
