#include "parseConfig.h"
#include <wordexp.h>
#include <wlr/util/log.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <lualib.h>
#include <lua.h>
#include <libgen.h>
#include <lauxlib.h>
#include "utils/gapUtils.h"
#include "translationLayer.h"
#include "tile/tileUtils.h"
#include "root.h"
#include "utils/parseConfigUtils.h"
#include "lib/actions/actions.h"
#include "stringop.h"
#include "workspace.h"
#include "server.h"

size_t rule_count;

int lib_reload_config(lua_State *L)
{
    for (int i = 0; i < server.options.tag_names.length; i++)
        free(wlr_list_pop(&server.options.tag_names));
    wlr_list_finish(&server.options.tag_names);
    server.options = get_default_options();

    destroy_workspaces();
    /* update_config(L); */
    create_workspaces(server.options.tag_names, default_layout);

    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    struct layout *lt = &ws->layout[0];

    // reconfigure clients
    struct client *c;
    wl_list_for_each(c, &clients, link) {
        c->bw = lt->options.tile_border_px;
    }

    printf("reload_config\n");
    arrange();
    return 0;
}
