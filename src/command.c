#include "command.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <wordexp.h>
#include <wlr/types/wlr_list.h>
#include "workspace.h"
#include "monitor.h"
#include "stringop.h"
#include "keybinding.h"

void execute_command(const char *_exec)
{
    // Split command list
    char *exec = strdup(_exec);
    char *head = exec;
    char matched_delim = ';';
    const char *cmd = argsep(&head, ";,", &matched_delim);

    for (; isspace(*cmd); ++cmd) {}

    if (strcmp(cmd, "") == 0) {
        printf("Ignoring empty command.\n");
        return;
    }
        printf("Handling command '%s'\n", cmd);
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

        // execute command
        if (strcmp(argv[0], "workspace") == 0) {
            bool handled = false;
            int i;
            for (i = 0; i < workspace_count(); i++) {
                if (strcmp(get_workspace(i)->name, argv[2]) == 0) {
                    handled = true;
                    break;
                }
            }
            if (handled) {
                if (key_state_has_modifiers(MOD_SHIFT)) {
                    focus_workspace(selected_monitor, get_workspace(i));
            } else {
                push_selected_workspace(selected_monitor, get_workspace(i));
            }
        }
    }
    free(argv);
}
