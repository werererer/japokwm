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

float overlay_color[4];
float text_color[4];
float sel_overlay_color[4];
float sel_text_color[4];

struct wlr_list tag_names;
struct rule *rules;
size_t rule_count;
struct mon_rule *monrules;
size_t monrule_count;

int repeat_rate;
int repeat_delay;

struct wlr_list tag_names;
char *termcmd;
struct layout *keys = NULL;
struct layout *buttons = NULL;

int update_config(lua_State *L)
{
    init_error_file();
    init_config(L);

    /* appearance */
    get_config_float_arr(L, overlay_color, "Overlay_color");
    get_config_float_arr(L, text_color, "Text_color");
    get_config_float_arr(L, sel_overlay_color, "Sel_overlay_color");
    get_config_float_arr(L, sel_text_color, "Sel_text_color");

    wlr_list_init(&tag_names);
    get_config_str_arr(L, &tag_names, "Tag_names");

    get_config_rule_arr(L, &rules, &rule_count, "Rules");
    get_config_mon_rule_arr(L, &monrules, &monrule_count, "Monrules");

    /* keyboard */
    repeat_rate = get_config_int(L, "Repeat_rate");
    repeat_delay = get_config_int(L, "Repeat_delay");
    default_layout = get_config_layout(L, "Default_layout");
    prev_layout = (struct layout) {
        .name = "",
        .symbol = "",
        .nmaster = 1,
        .n = 0,
        .lua_layout_index = 0,
        .lua_layout_copy_data_index = 0,
        .lua_box_data_index = 0,
        .arrange_by_focus = false,
    };

    /* commands */
    termcmd = get_config_str(L, "Termcmd");

    close_error_file();
    return 0;
}

int reload_config(lua_State *L)
{
    for (int i = 0; i < tag_names.length; i++)
        free(wlr_list_pop(&tag_names));
    wlr_list_finish(&tag_names);

    destroy_workspaces();
    update_config(L);
    create_workspaces(tag_names, default_layout);

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
