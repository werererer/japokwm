#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <stdint.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <json.h>
#include "stringop.h"
#include "ipc-client.h"
#include "log.h"

static bool success_object(json_object *result) {
    json_object *success;

    if (!json_object_object_get_ex(result, "success", &success)) {
        return true;
    }

    return json_object_get_boolean(success);
}

// Iterate results array and return false if any of them failed
static bool success(json_object *r, bool fallback) {
    if (!json_object_is_type(r, json_type_array)) {
        if (json_object_is_type(r, json_type_object)) {
            return success_object(r);
        }
        return fallback;
    }

    size_t results_len = json_object_array_length(r);
    if (!results_len) {
        return fallback;
    }

    for (size_t i = 0; i < results_len; ++i) {
        json_object *result = json_object_array_get_idx(r, i);

        if (!success_object(result)) {
            return false;
        }
    }

    return true;
}

static void pretty_print_cmd(json_object *r) {
    json_object *error;
    bool is_error_defined = json_object_object_get_ex(r, "error", &error);
    if (success_object(r)) {
        printf("%s\n", json_object_get_string(error));
    } else {
        if (!is_error_defined) {
            printf("An unknown error occurred");
        } else {
            printf("Error: %s\n", json_object_get_string(error));
        }
    }
}

static void pretty_print(int type, json_object *resp) {
    printf("%s\n", json_object_to_json_string_ext(resp,
                JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED));

    json_object *obj;
    size_t len = json_object_array_length(resp);
    for (size_t i = 0; i < len; ++i) {
        obj = json_object_array_get_idx(resp, i);
        pretty_print_cmd(obj);
    }
}

int main(int argc, char **argv) {
    static bool quiet = false;
    char *socket_path = NULL;
    char *cmdtype = NULL;

    log_init(JAPOKWM_INFO, NULL);

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"pretty", no_argument, NULL, 'p'},
        {"quiet", no_argument, NULL, 'q'},
        {"config", required_argument, NULL, 'c'},
        {"socket", required_argument, NULL, 's'},
        {"type", required_argument, NULL, 't'},
        {"version", no_argument, NULL, 'v'},
        {0, 0, 0, 0}
    };

    const char *usage =
        "Usage: japokmsg [options] [message]\n"
        "\n"
        "  -h, --help             Show help message and quit.\n"
        "  -m, --monitor          Monitor until killed (-t SUBSCRIBE only)\n"
        "  -q, --quiet            Be quiet.\n"
        "  -s, --socket <socket>  Use the specified socket.\n"
        "  -t, --type <type>      Specify the message type.\n"
        "  -v, --version          Show the version number and quit.\n";

    int c;
    while (1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "hmqs:t:v", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'q': // Quiet
                quiet = true;
                break;
            case 's': // Socket
                socket_path = strdup(optarg);
                break;
            case 't': // Type
                cmdtype = strdup(optarg);
                break;
            case 'v':
                fprintf(stdout, "japokmsg version " JAPOKWM_VERSION "\n");
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr, "%s", usage);
                exit(EXIT_FAILURE);
        }
    }

    if (!cmdtype) {
        cmdtype = strdup("command");
    }
    if (!socket_path) {
        socket_path = get_socketpath();
        if (!socket_path) {
            if (quiet) {
                exit(EXIT_FAILURE);
            }
            sway_abort("Unable to retrieve socket path");
        }
    }

    uint32_t type = IPC_COMMAND;

    if (strcasecmp(cmdtype, "command") == 0) {
        type = IPC_COMMAND;
    }

    free(cmdtype);

    char *command = NULL;
    if (optind < argc) {
        command = join_args(argv + optind, argc - optind);
    } else {
        command = strdup("");
    }

    int ret = 0;
    int socketfd = ipc_open_socket(socket_path);
    struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
    ipc_set_recv_timeout(socketfd, timeout);
    uint32_t len = strlen(command);
    char *resp = ipc_single_command(socketfd, type, command, &len);

    printf("works\n");
    // pretty print the json
    json_object *obj = json_tokener_parse(resp);
    if (obj == NULL) {
        if (!quiet) {
            fprintf(stderr, "ERROR: Could not parse json response from ipc. "
                    "This is a bug in japokwm.");
            printf("%s\n", resp);
        }
        ret = 1;
    } else {
        if (!success(obj, true)) {
            ret = 2;
        }
        printf("quiet: %i\n", quiet);
        if (!quiet) {
            printf("pretty print\n");
            pretty_print(type, obj);
        }
        json_object_put(obj);
    }
    free(command);
    free(resp);
    close(socketfd);

    free(socket_path);
    return ret;
}
