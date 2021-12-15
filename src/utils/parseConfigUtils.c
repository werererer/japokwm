#include "utils/parseConfigUtils.h"

#include <assert.h>
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
#include <fts.h>

#include "tile/tileUtils.h"
#include "options.h"
#include "server.h"
#include "utils/writeFile.h"
#include "stringop.h"
#include "utils/coreUtils.h"
#include "rules/mon_rule.h"
#include "workspace.h"
#include "rules/rule.h"
#include "translationLayer.h"
#include "ipc-server.h"
#include "tagset.h"

static const char *plugin_relative_paths[] = {
    "autoload",
    "plugins",
};

static const char *config_file = "init.lua";
static const char *error_file = "init.err";
static int error_fd = -1;

// returns 0 upon success and 1 upon failure
int load_file(lua_State *L, const char *file)
{
    if (!file_exists(file)) {
        return EXIT_FAILURE;
    }

    if (luaL_loadfile(L, file)) {
        const char *errmsg = luaL_checkstring(L, -1);
        handle_error(errmsg);
        lua_pop(L, 1);
        return EXIT_FAILURE;
    }

    int ret = lua_call_safe(L, 0, 0, 0);

    return ret;
}

GPtrArray *create_default_config_paths()
{
    GPtrArray *config_paths = g_ptr_array_new();
    g_ptr_array_add(config_paths, "$XDG_CONFIG_HOME/japokwm/");
    g_ptr_array_add(config_paths, "$HOME/.config/japokwm/");
    g_ptr_array_add(config_paths, "/etc/japokwm/");
    return config_paths;
}

GPtrArray *create_default_user_data_paths()
{
    GPtrArray *user_data_paths = g_ptr_array_new();
    g_ptr_array_add(user_data_paths, "/usr/local/share/japokwm");
    g_ptr_array_add(user_data_paths, "/usr/share/japokwm");
    return user_data_paths;
}

GPtrArray *create_default_layout_paths()
{
    GPtrArray *layout_paths = g_ptr_array_new();
    g_ptr_array_add(layout_paths, "$XDG_CONFIG_HOME/japokwm/");
    g_ptr_array_add(layout_paths, "$HOME/.config/japokwm/");
    g_ptr_array_add(layout_paths, "/etc/japokwm/");
    g_ptr_array_add(layout_paths, "/usr/local/share/japokwm");
    g_ptr_array_add(layout_paths, "/usr/share/japokwm");
    return layout_paths;
}

char *get_config_file(GPtrArray *paths, const char *file)
{
    for (size_t i = 0; i < paths->len; ++i) {
        char *path = strdup(g_ptr_array_index(paths, i));
        join_path(&path, file);
        expand_path(&path);
        if (file_exists(path))
            return path;
        free(path);
    }
    return NULL;
}

char *get_config_layout_path()
{
    return get_config_file(server.layout_paths, "layouts");
}

char *get_config_dir(GPtrArray *paths, const char *file)
{
    char *abs_file = get_config_file(paths, file);
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

static int load_plugin_paths(lua_State *L)
{
    char *base_path = get_config_dir(server.config_paths, config_file);
    if (!base_path) {
        return 1;
    }
    for (int i = 0; i < LENGTH(plugin_relative_paths); i++) {
        char *base = strdup(base_path);
        const char *path = plugin_relative_paths[i];
        join_path(&base, path);
        join_path(&base, "?");
        join_path(&base, "init.lua");
        append_to_lua_path(L, base);
        free(base);
    }
    free(base_path);
    return 1;
}

// returns 0 upon success and 1 upon failure
static int load_default_config(lua_State *L)
{
    char *file_path = get_config_file(server.config_paths, config_file);
    if (!file_path)
        return EXIT_FAILURE;

    int success = load_file(L, file_path);
    free(file_path);

    return success;
}

// returns 0 upon success and 1 upon failure
int load_config(lua_State *L)
{
    int success = 0;
    init_global_config_variables(L);

    if (server.config_file != NULL && strcmp(server.config_file, "") != 0) {
        debug_print("load file\n");
        success = load_file(L, server.config_file);
    } else {
        debug_print("load_default_config\n");
        success = load_default_config(L);
    }
    return success;
}

void load_default_lua_config(lua_State *L)
{
    options_reset(server.default_layout->options);

    list_clear(server.default_layout->options->tag_names, NULL);
    GPtrArray *tagnames = server.default_layout->options->tag_names;
    g_ptr_array_add(tagnames, "0:1");
    g_ptr_array_add(tagnames, "1:2");
    g_ptr_array_add(tagnames, "2:3");
    g_ptr_array_add(tagnames, "3:4");
    server.default_layout->name = "";
    load_workspaces(server_get_workspaces(), tagnames);

    bitset_set(server.previous_bitset, server.previous_workspace);

    workspaces_remove_loaded_layouts(server_get_workspaces());

    // FIXME: you have to reconnect your workspaces because workspaces may have
    // been destroyed that belong to the tagset this may lead to segfaults
    for (int i = 0; i < server_get_workspace_count(); i++) {
        struct workspace *ws = get_workspace(i);
        set_default_layout(ws);
    }

    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct workspace *ws = monitor_get_active_workspace(m);
        tagset_focus_tags(ws, ws->prev_workspaces);
    }

    ipc_event_workspace();

    arrange();
}

// returns 0 upon success and 1 upon failure
int init_utils(lua_State *L)
{
    init_global_config_variables(L);
    load_plugin_paths(L);
    const char *tile_file = "tile.lua";
    char *config_dir = get_config_dir(server.user_data_paths, tile_file);

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
    char *ef_dir = NULL;
    if (server.custom_path) {
        if (file_exists(server.custom_path)) {
            ef_dir = strdup(server.custom_path);
        } else {
            ef_dir = strdup(server.error_path);
        }
    } else {
        ef_dir = strdup(server.error_path);
    }

    mkdir(ef_dir, 0777);
    char *ef = strdup(ef_dir);
    join_path(&ef, error_file);
    error_fd = open(ef, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    free(ef);
    free(ef_dir);
}

void close_error_file()
{
    assert(error_fd >= 0);
    close(error_fd);
    error_fd = -1;
}

int lua_call_safe(lua_State *L, int nargs, int nresults, int msgh)
{
    int lua_status = lua_pcall(L, nargs, nresults, msgh);
    if (lua_status != LUA_OK) {
        const char *errmsg = luaL_checkstring(L, -1);
        handle_error(errmsg);
        lua_pop(L, 1);
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
    strip_whitespace(msg);
    if (strcmp(msg, "") == 0) {
        goto exit_cleanup;
    }

    notify_init(msg);
    NotifyNotification* n = notify_notification_new("Japokwm.message: ", 
            msg,
            0);
    notify_notification_set_timeout(n, 10000); // 10 seconds

    /* NOTE: this causes a lot of thread warnings in valgrind's helgrind tool. I
     * am pretty sure that this application is not of fault(or I hope so :O)
     * because similiar errors can be reproduced by calling
     * `valgrind -tool=helgrind notify-send "test"`. Btw I also tested this by
     * removing the thread that calls this function with the same errors
     * reappearing */
    if (!notify_notification_show(n, 0))
    {
        printf("showing notification failed!\n");
    }

exit_cleanup:
    free(msg);
    return NULL;
}

void notify_msg(const char *msg)
{
    if (!msg)
        return;
    // we need to call this in a thread because sometimes notifications hang or
    // so I experienced.
    pthread_t thread;
    pthread_create(&thread, NULL, _notify_msg, strdup(msg));
    pthread_detach(thread);
}

void write_to_error_file(const char *msg)
{
    if (error_fd < 0)
        return;

    write_to_file(error_fd, msg);
}

void write_line_to_error_file(const char *msg)
{
    if (error_fd < 0)
        return;

    write_to_file(error_fd, msg);
    write_to_file(error_fd, "\n");
}

void handle_error(const char *msg)
{
    notify_msg(msg);

    printf("%s\n", msg);
    load_default_lua_config(L);

    // if error file not initialized
    if (error_fd < 0)
        return;

    write_line_to_error_file(msg);
}

void handle_warning(void *user_data, const char *msg, int i)
{
    notify_msg(msg);

    char *final_message = g_strconcat("WARNING: ", msg, "\n", NULL);
    printf("%s", final_message);

    // if error file not initialized
    if (error_fd < 0)
        return;

    write_to_file(error_fd, msg);
    write_to_file(error_fd, "\n");
    free(final_message);
}

const char *get_config_str(lua_State *L, int idx)
{
    if (!lua_isstring(L, idx)) {
        return "";
    }
    const char *str = luaL_checkstring(L, idx);
    return str;
}

const char *get_config_array_str(lua_State *L, const char *name, size_t i)
{
    lua_rawgeti(L, -1, i);
    const char *str = get_config_str(L, -1);
    lua_pop(L, 1);
    return str;
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

static int get_config_array_func(lua_State *L, const char *name, int i)
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

struct rule *get_config_rule(lua_State *L)
{
    lua_getfield(L, -1, "class");
    const char *id = get_config_str(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "name");
    const char *title = get_config_str(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "callback");
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);

    lua_pop(L, 1);

    struct rule *rule = create_rule(id, title, lua_func_ref);
    return rule;
}

struct mon_rule *get_config_mon_rule(lua_State *L)
{
    lua_getfield(L, -1, "output");
    const char *output_name = get_config_str(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, -1, "callback");
    int lua_func_ref = 0;
    lua_ref_safe(L, LUA_REGISTRYINDEX, &lua_func_ref);

    struct mon_rule *mon_rule = create_mon_rule(output_name, lua_func_ref);
    return mon_rule;
}
