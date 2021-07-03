#include "command.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <wordexp.h>
#include <wlr/types/wlr_list.h>
#include <wlr/backend/multi.h>
#include <json-c/json.h>
#include <wlr/backend/wayland.h>
#include <wlr/backend/headless.h>

#include "workspace.h"
#include "monitor.h"
#include "stringop.h"
#include "keybinding.h"
#include "server.h"

/* Keep alphabetized */
static struct cmd_handler handlers[] = {
    /* { "assign", cmd_assign }, */
    /* { "bar", cmd_bar }, */
    /* { "bindcode", cmd_bindcode }, */
    /* { "bindswitch", cmd_bindswitch }, */
    /* { "bindsym", cmd_bindsym }, */
    /* { "client.background", cmd_client_noop }, */
    /* { "client.focused", cmd_client_focused }, */
    /* { "client.focused_inactive", cmd_client_focused_inactive }, */
    /* { "client.placeholder", cmd_client_noop }, */
    /* { "client.unfocused", cmd_client_unfocused }, */
    /* { "client.urgent", cmd_client_urgent }, */
    /* { "default_border", cmd_default_border }, */
    /* { "default_floating_border", cmd_default_floating_border }, */
    /* { "exec", cmd_exec }, */
    /* { "exec_always", cmd_exec_always }, */
    /* { "floating_maximum_size", cmd_floating_maximum_size }, */
    /* { "floating_minimum_size", cmd_floating_minimum_size }, */
    /* { "floating_modifier", cmd_floating_modifier }, */
    /* { "focus", cmd_focus }, */
    /* { "focus_follows_mouse", cmd_focus_follows_mouse }, */
    /* { "focus_on_window_activation", cmd_focus_on_window_activation }, */
    /* { "focus_wrapping", cmd_focus_wrapping }, */
    /* { "font", cmd_font }, */
    /* { "for_window", cmd_for_window }, */
    /* { "force_display_urgency_hint", cmd_force_display_urgency_hint }, */
    /* { "force_focus_wrapping", cmd_force_focus_wrapping }, */
    /* { "fullscreen", cmd_fullscreen }, */
    /* { "gaps", cmd_gaps }, */
    /* { "hide_edge_borders", cmd_hide_edge_borders }, */
    /* { "input", cmd_input }, */
    /* { "mode", cmd_mode }, */
    /* { "mouse_warping", cmd_mouse_warping }, */
    /* { "new_float", cmd_new_float }, */
    /* { "new_window", cmd_new_window }, */
    /* { "no_focus", cmd_no_focus }, */
    /* { "output", cmd_output }, */
    /* { "popup_during_fullscreen", cmd_popup_during_fullscreen }, */
    /* { "seat", cmd_seat }, */
    /* { "set", cmd_set }, */
    /* { "show_marks", cmd_show_marks }, */
    /* { "smart_borders", cmd_smart_borders }, */
    /* { "smart_gaps", cmd_smart_gaps }, */
    /* { "tiling_drag", cmd_tiling_drag }, */
    /* { "tiling_drag_threshold", cmd_tiling_drag_threshold }, */
    /* { "title_align", cmd_title_align }, */
    /* { "titlebar_border_thickness", cmd_titlebar_border_thickness }, */
    /* { "titlebar_padding", cmd_titlebar_padding }, */
    /* { "unbindcode", cmd_unbindcode }, */
    /* { "unbindswitch", cmd_unbindswitch }, */
    /* { "unbindsym", cmd_unbindsym }, */
    /* { "workspace", cmd_workspace }, */
    /* { "workspace_auto_back_and_forth", cmd_ws_auto_back_and_forth }, */
};

/* Config-time only commands. Keep alphabetized */
static struct cmd_handler config_handlers[] = {
    /* { "default_orientation", cmd_default_orientation }, */
    /* { "include", cmd_include }, */
    /* { "swaybg_command", cmd_swaybg_command }, */
    /* { "swaynag_command", cmd_swaynag_command }, */
    /* { "workspace_layout", cmd_workspace_layout }, */
    /* { "xwayland", cmd_xwayland }, */
};

static void create_output(struct wlr_backend *backend, void *data) {
    bool *done = data;
    if (*done) {
        return;
    }

    if (wlr_backend_is_wl(backend)) {
        wlr_wl_output_create(backend);
        *done = true;
    } else if (wlr_backend_is_headless(backend)) {
        wlr_headless_add_output(backend, 1920, 1080);
        *done = true;
    }
/* #if WLR_HAS_X11_BACKEND */
/*  else if (wlr_backend_is_x11(backend)) { */
/*      wlr_x11_output_create(backend); */
/*      *done = true; */
/*  } */
/* #endif */
}

struct cmd_results *cmd_create_output(int argc, char **argv) {
    printf("create output\n");
    if (!wlr_backend_is_multi(server.backend))
        printf("Expected a multi backend\n");

    bool done = false;
    wlr_multi_for_each_backend(server.backend, create_output, &done);

    if (!done) {
        return cmd_results_new(CMD_INVALID,
            "Can only create outputs for Wayland, X11 or headless backends");
    }

    return cmd_results_new(CMD_SUCCESS, NULL);
}

/* Runtime-only commands. Keep alphabetized */
static struct cmd_handler command_handlers[] = {
    /* { "border", cmd_border }, */
    { "create_output", cmd_create_output },
    /* { "exit", cmd_exit }, */
    /* { "floating", cmd_floating }, */
    /* { "fullscreen", cmd_fullscreen }, */
    /* { "inhibit_idle", cmd_inhibit_idle }, */
    /* { "kill", cmd_kill }, */
    /* { "layout", cmd_layout }, */
    /* { "mark", cmd_mark }, */
    /* { "max_render_time", cmd_max_render_time }, */
    /* { "move", cmd_move }, */
    /* { "nop", cmd_nop }, */
    /* { "opacity", cmd_opacity }, */
    /* { "reload", cmd_reload }, */
    /* { "rename", cmd_rename }, */
    /* { "resize", cmd_resize }, */
    /* { "scratchpad", cmd_scratchpad }, */
    /* { "shortcuts_inhibitor", cmd_shortcuts_inhibitor }, */
    /* { "split", cmd_split }, */
    /* { "splith", cmd_splith }, */
    /* { "splitt", cmd_splitt }, */
    /* { "splitv", cmd_splitv }, */
    /* { "sticky", cmd_sticky }, */
    /* { "swap", cmd_swap }, */
    /* { "title_format", cmd_title_format }, */
    /* { "unmark", cmd_unmark }, */
    /* { "urgent", cmd_urgent }, */
};


static int handler_compare(const void *_a, const void *_b) {
    const struct cmd_handler *a = _a;
    const struct cmd_handler *b = _b;
    return strcasecmp(a->command, b->command);
}

struct cmd_handler *find_handler(char *line, struct cmd_handler *handlers,
        size_t handlers_size) {
    if (!handlers || !handlers_size) {
        return NULL;
    }
    struct cmd_handler query = { .command = line };
    return bsearch(&query, handlers,
            handlers_size / sizeof(struct cmd_handler),
            sizeof(struct cmd_handler), handler_compare);
}

static struct cmd_handler *find_handler_ex(char *line,
        struct cmd_handler *config_handlers, size_t config_handlers_size,
        struct cmd_handler *command_handlers, size_t command_handlers_size,
        struct cmd_handler *handlers, size_t handlers_size) {
    struct cmd_handler *handler = 
        find_handler(line, command_handlers, command_handlers_size);
    return handler ? handler : find_handler(line, handlers, handlers_size);
}

static struct cmd_handler *find_core_handler(char *line) {
    return find_handler_ex(line, config_handlers, sizeof(config_handlers),
            command_handlers, sizeof(command_handlers),
            handlers, sizeof(handlers));
}

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

struct wlr_list execute_command(char *_exec, struct wlr_seat *seat,
        struct container *con) {
    char *cmd;
    char matched_delim = ';';
    struct wlr_list containers;
    wlr_list_init(&containers);

    char *exec = strdup(_exec);
    char *head = exec;
    struct wlr_list res_list;
    wlr_list_init(&res_list);

    if (!exec) {
        return res_list;
    }

    do {
        for (; isspace(*head); ++head) {}
        // Extract criteria (valid for this command list only).
        /* if (matched_delim == ';') { */
        /*     config->handler_context.using_criteria = false; */
        /*     if (*head == '[') { */
        /*         char *error = NULL; */
        /*         struct criteria *criteria = criteria_parse(head, &error); */
        /*         if (!criteria) { */
        /*             list_add(res_list, */
        /*                     cmd_results_new(CMD_INVALID, "%s", error)); */
        /*             free(error); */
        /*             goto cleanup; */
        /*         } */
        /*         list_free(containers); */
        /*         containers = criteria_get_containers(criteria); */
        /*         head += strlen(criteria->raw); */
        /*         criteria_destroy(criteria); */
        /*         config->handler_context.using_criteria = true; */
        /*         // Skip leading whitespace */
        /*         for (; isspace(*head); ++head) {} */
        /*     } */
        /* } */
        // Split command list
        cmd = argsep(&head, ";,", &matched_delim);
        for (; isspace(*cmd); ++cmd) {}

        if (strcmp(cmd, "") == 0) {
            /* sway_log(SWAY_INFO, "Ignoring empty command."); */
            continue;
        }
        /* sway_log(SWAY_INFO, "Handling command '%s'", cmd); */
        //TODO better handling of argv
        int argc;
        char **argv = split_args(cmd, &argc);
        if (strcmp(argv[0], "exec") != 0 &&
                strcmp(argv[0], "exec_always") != 0 &&
                strcmp(argv[0], "mode") != 0) {
            for (int i = 1; i < argc; ++i) {
                if (*argv[i] == '\"' || *argv[i] == '\'') {
                    strip_quotes(argv[i]);
                }
            }
        }
        struct cmd_handler *handler = find_core_handler(argv[0]);
        if (!handler) {
            wlr_list_push(&res_list, cmd_results_new(CMD_INVALID,
                    "Unknown/invalid command '%s'", argv[0]));
            free_argv(argc, argv);
            goto cleanup;
        }

        // Var replacement, for all but first argument of set
        /* for (int i = handler->handle == cmd_set ? 2 : 1; i < argc; ++i) { */
        /*     argv[i] = do_var_replacement(argv[i]); */
        /* } */

        /* if (!config->handler_context.using_criteria) { */
        /*     // The container or workspace which this command will run on. */
        /*     struct sway_node *node = con ? &con->node : */
        /*             seat_get_focus_inactive(seat, &root->node); */
        /*     set_config_node(node); */
            struct cmd_results *res = handler->handle(argc-1, argv+1);
            wlr_list_push(&res_list, res);
            if (res->status == CMD_INVALID) {
                free_argv(argc, argv);
                goto cleanup;
            }
        /* } else if (containers->length == 0) { */
        /*     wlr_list_push(&res_list, */
        /*             cmd_results_new(CMD_FAILURE, "No matching node.")); */
        /* } else { */
        /*     struct cmd_results *fail_res = NULL; */
        /*     for (int i = 0; i < containers->length; ++i) { */
        /*         struct sway_container *container = containers->items[i]; */
        /*         set_config_node(&container->node); */
        /*         struct cmd_results *res = handler->handle(argc-1, argv+1); */
        /*         if (res->status == CMD_SUCCESS) { */
        /*             free_cmd_results(res); */
        /*         } else { */
        /*             // last failure will take precedence */
        /*             if (fail_res) { */
        /*                 free_cmd_results(fail_res); */
        /*             } */
        /*             fail_res = res; */
        /*             if (res->status == CMD_INVALID) { */
        /*                 wlr_list_push(&res_list, fail_res); */
        /*                 free_argv(argc, argv); */
        /*                 goto cleanup; */
        /*             } */
        /*         } */
        /*     } */
            /* wlr_list_push(&res_list, */
            /*         fail_res ? fail_res : cmd_results_new(CMD_SUCCESS, NULL)); */
        /* } */
        free_argv(argc, argv);
    } while(head);
cleanup:
    free(exec);
    wlr_list_finish(&containers);
    return res_list;
}

char *cmd_results_to_json(struct wlr_list res_list) {
    json_object *result_array = json_object_new_array();
    for (int i = 0; i < res_list.length; ++i) {
        struct cmd_results *results = res_list.items[i];
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
    }
    const char *json = json_object_to_json_string(result_array);
    char *res = strdup(json);
    json_object_put(result_array);
    return res;
}
