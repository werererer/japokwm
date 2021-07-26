#include "command/commands.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <wlr/backend/multi.h>
#include <wlr/backend/wayland.h>
#include <wlr/backend/headless.h>

#include "monitor.h"
#include "command.h"
#include "lib/actions/actions.h"
#include "server.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"

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

struct cmd_results *cmd_create_output(int argc, char **argv)
{
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
