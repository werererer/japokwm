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

int update_config(lua_State *L)
{
    printf("update_config\n");
    init_error_file();
    init_config(L);

    /* default_layout = get_config_layout(L, "Default_layout"); */
    prev_layout = (struct layout) {
        .name = "",
        .symbol = "",
        .nmaster = 1,
        .n = 0,
        .lua_layout_index = 0,
        .lua_layout_copy_data_index = 0,
        .lua_box_data_index = 0,
    };

    close_error_file();
    return 0;
}

int reload_config(lua_State *L)
{
    for (int i = 0; i < server.options.tag_names.length; i++)
        free(wlr_list_pop(&server.options.tag_names));
    wlr_list_finish(&server.options.tag_names);

    destroy_workspaces();
    update_config(L);
    create_workspaces(server.options.tag_names, default_layout);

    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws;
    struct layout *lt = &ws->layout;

    // reconfigure clients
    struct client *c;
    wl_list_for_each(c, &clients, link) {
        c->bw = lt->options.border_px;
    }

    printf("reload_config\n");
    arrange();
    return 0;
}
