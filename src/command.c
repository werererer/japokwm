#include "command.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <wordexp.h>
#include <wlr/types/wlr_list.h>
#include "tagset.h"
#include "monitor.h"
#include "stringop.h"
#include "keybinding.h"

void executeCommand(const char *_exec)
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
        struct tagset *tagset = selected_monitor->tagset;
        int i;
        for (i = 0; i < tagset->tags.length; i++) {
            if (strcmp(get_tag_from_tagset(tagset, i)->name, argv[2]) == 0) {
                handled = true;
                break;
            }
        }
        if (handled) {
            if (key_state_has_modifiers(MOD_SHIFT)) {
                toggle_add_tag(tagset, position_to_flag(i));
            } else {
                push_seleceted_tags(tagset, position_to_flag(i));
                tagset->focusedTag = i;
            }
        }
    }
    free(argv);
}
