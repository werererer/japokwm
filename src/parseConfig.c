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

bool sloppyFocus;
int borderPx;
float borderColor[4];
float focusColor[4];
float overlayColor[4];
float textColor[4];
float selOverlayColor[4];
float selTextColor[4];

struct wlr_list tag_names;
struct rule rules[MAXLEN];

//MonitorRule monrules[MAXLEN];

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
    sloppyFocus = get_config_bool(L, "sloppyFocus");
    borderPx = get_config_int(L, "borderPx");

    /* gaps */
    inner_gap = get_config_int(L, "innerGap");
    outer_gap = get_config_int(L, "outerGap");
    configure_gaps(&inner_gap, &outer_gap);

    /* appearance */
    get_config_float_arr(L, root.color, "rootColor");
    get_config_float_arr(L, borderColor, "borderColor");
    get_config_float_arr(L, focusColor, "focusColor");
    get_config_float_arr(L, overlayColor, "overlayColor");
    get_config_float_arr(L, textColor, "textColor");
    get_config_float_arr(L, selOverlayColor, "overlayColor");
    get_config_float_arr(L, selTextColor, "textColor");

    wlr_list_init(&tag_names);
    get_config_str_arr(L, &tag_names, "tagNames");
    get_config_rule_arr(L, rules, "rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeat_rate = get_config_int(L, "repeatRate");
    repeat_delay = get_config_int(L, "repeatDelay");
    defaultLayout = get_config_layout(L, "defaultLayout");
    prev_layout = (struct layout){.symbol = "", .funcId = 0};

    /* commands */
    termcmd = get_config_str(L, "termcmd");
    get_config_key_arr(L, keys, "keys");
    get_config_key_arr(L, buttons, "buttons");

    close_error_file();
    return 0;
}

int reloadConfig(lua_State *L)
{
    for (int i = 0; i < tag_names.length; i++)
        free(wlr_list_pop(&tag_names));
    wlr_list_finish(&tag_names);

    destroy_workspaces();
    update_config(L);
    create_workspaces(tag_names);

    // reconfigure clients
    struct client *c = NULL;
    wl_list_for_each(c, &clients, link) {
        c->bw = borderPx;
    }

    lua_pushboolean(L, true);
    arrange_this(L);
    return 0;
}
