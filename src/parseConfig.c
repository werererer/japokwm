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

bool sloppy_focus;
int border_px;
float root_color[4];
float border_color[4];
float focus_color[4];
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
int inner_gap;
int outer_gap;

struct wlr_list tag_names;
char *termcmd;
Key *keys = NULL;
Key *buttons = NULL;

int update_config(lua_State *L)
{
    init_error_file();
    init_config(L);
    sloppy_focus = get_config_bool(L, "Sloppy_focus");
    border_px = get_config_int(L, "Border_px");

    /* gaps */
    inner_gap = get_config_int(L, "Inner_gap");
    outer_gap = get_config_int(L, "Outer_gap");
    configure_gaps(&inner_gap, &outer_gap);

    /* appearance */
    get_config_float_arr(L, root_color, "Root_color");
    get_config_float_arr(L, border_color, "Border_color");
    get_config_float_arr(L, focus_color, "Focus_color");
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

    // reconfigure clients
    struct client *c = NULL;
    wl_list_for_each(c, &clients, link) {
        c->bw = border_px;
    }

    printf("reload_config\n");
    arrange();
    return 0;
}
