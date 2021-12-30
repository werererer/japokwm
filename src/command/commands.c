#include "command/commands.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wlr/backend/multi.h>
#include <wlr/backend/wayland.h>
#include <wlr/backend/headless.h>
#include <wlr/backend/x11.h>
#include <wlr/util/log.h>

#include "output.h"
#include "command.h"
#include "server.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"

struct cmd_results *cmd_create_output(int argc, char **argv)
{
    if (!wlr_backend_is_multi(server.backend)) {
        printf("Expected a multi backend\n");
    }

    bool done = false;
    wlr_multi_for_each_backend(server.backend, create_output, &done);

    if (!done) {
        return cmd_results_new(CMD_INVALID,
            "Can only create outputs for Wayland, X11 or headless backends");
    }

    return cmd_results_new(CMD_SUCCESS, NULL);
}
