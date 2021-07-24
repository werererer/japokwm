#include "command.h"

#include <json-c/json.h>

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

char *cmd_results_to_json(GPtrArray *res_list) {
    json_object *result_array = json_object_new_array();
    for (int i = 0; i < res_list->len; ++i) {
        struct cmd_results *results = g_ptr_array_index(res_list, i);
        json_object *root = json_object_new_object();
        json_object_object_add(root, "success",
                json_object_new_boolean(false));
        if (results->error) {
            json_object_object_add(root, "parse_error",
                    json_object_new_boolean(results->status == CMD_INVALID));
            json_object_object_add(
                    root, "error", json_object_new_string(results->error));
        }
        json_object_array_add(result_array, root);
    }
    const char *json = json_object_to_json_string(result_array);
    char *res = strdup(json);
    json_object_put(result_array);
    return res;
}
