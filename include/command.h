#ifndef COMMAND_H
#define COMMAND_H

#include <wlr/types/wlr_seat.h>

struct container;

typedef struct cmd_results *sway_cmd(int argc, char **argv);

struct cmd_handler {
    char *command;
    sway_cmd *handle;
};

/**
 * Indicates the result of a command's execution.
 */
enum cmd_status {
    CMD_SUCCESS,        /**< The command was successful */
    CMD_FAILURE,        /**< The command resulted in an error */
    CMD_INVALID,        /**< Unknown command or parser error */
    CMD_DEFER,      /**< Command execution deferred */
    CMD_BLOCK,
    CMD_BLOCK_COMMANDS,
    CMD_BLOCK_END
};

/**
 * Stores the result of executing a command.
 */
struct cmd_results {
    enum cmd_status status;
    /**
     * Human friendly error message, or NULL on success
     */
    char *error;
};

enum expected_args {
    EXPECTED_AT_LEAST,
    EXPECTED_AT_MOST,
    EXPECTED_EQUAL_TO
};

/**
 * Allocates a cmd_results object.
 */
struct cmd_results *cmd_results_new(enum cmd_status status, const char *error, ...);
/**
 * Frees a cmd_results object.
 */
void free_cmd_results(struct cmd_results *results);

struct cmd_results *execute_command(
        char *cmd,
        struct wlr_seat *seat,
        struct container *con);
char *cmd_results_to_json(struct cmd_results *results);

struct cmd_results *cmd_eval(const char *cmd);

#endif /* COMMAND_H */
