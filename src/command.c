#include "command.h"

#include <json-c/json.h>

#include "utils/parseConfigUtils.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"

struct cmd_results *cmd_results_new(enum cmd_status status,
        const char *format, ...) {
    struct cmd_results *results = malloc(sizeof(struct cmd_results));
    if (!results) {
        /* sway_log(SWAY_ERROR, "Unable to allocate command results"); */
        return NULL;
    }
    results->status = status;
    if (format) {
        char *error = malloc(256);
        va_list args;
        va_start(args, format);
        if (error) {
            vsnprintf(error, 256, format, args);
        }
        va_end(args);
        results->error = error;
    } else {
        results->error = NULL;
    }
    return results;
}

void free_cmd_results(struct cmd_results *results) {
    if (results->error) {
        free(results->error);
    }
    free(results);
}

struct cmd_results *cmd_eval(const char *cmd)
{
    init_local_config_variables(L, server_get_selected_layout());

    // load is the equivalent to eval and was introduced in lua 5.2
    lua_getglobal_safe(L, "load");
    lua_pushstring(L, cmd);
    lua_call_safe(L, 1, 1, 0);
    // load returns a function pointer to the evaluated expression now we have
    // to call this function
    int lua_status = lua_pcall(L, 0, 1, 0);
    if (lua_status != LUA_OK) {
        const char *errmsg = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        return cmd_results_new(CMD_FAILURE, errmsg);
    }

    const char *text = "";
    if (!lua_isnil(L, -1)) {
        text = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    return cmd_results_new(CMD_SUCCESS, text);
}

struct cmd_results *execute_command(char *cmd, struct wlr_seat *seat,
        struct container *con) {
    struct cmd_results *res = cmd_eval(cmd);
    arrange();
    return res;
}

char *cmd_results_to_json(struct cmd_results *results) {
    json_object *result_array = json_object_new_array();

    json_object *root = json_object_new_object();
    json_object_object_add(root, "success",
            json_object_new_boolean(results->status == CMD_SUCCESS));
    if (results->error) {
        json_object_object_add(root, "parse_error",
                json_object_new_boolean(results->status == CMD_INVALID));
        json_object_object_add(
                root, "error", json_object_new_string(results->error));
    }
    json_object_array_add(result_array, root);

    const char *json = json_object_to_json_string(result_array);
    char *res = strdup(json);
    json_object_put(result_array);
    return res;
}
