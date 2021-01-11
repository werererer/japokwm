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
struct rule rules[MAXLEN];

struct mon_rule monrules[MAXLEN];

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
    printf("start\n");
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
    get_config_rule_arr(L, rules, "Rules");

    /* monitors */
    get_config_mon_rule_arr(L, monrules, "Monrules");

    /* keyboard */
    repeat_rate = get_config_int(L, "Repeat_rate");
    repeat_delay = get_config_int(L, "Repeat_delay");
    default_layout = get_config_layout(L, "Default_layout");
    prev_layout = (struct layout) {
        .name = "",
        .symbol = "",
        .lua_func_index = 0,
        .nmaster = 1,
        .n = 0,
        .lua_index = 0,
    };

    /* commands */
    termcmd = get_config_str(L, "Termcmd");
    get_config_key_arr(L, keys, "Keys");
    get_config_key_arr(L, buttons, "Buttons");

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

    lua_pushboolean(L, true);
    arrange_this(L);
    return 0;
}
