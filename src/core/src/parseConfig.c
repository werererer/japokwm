#include "parseConfig.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include "tile/tileUtils.h"
#include "utils/gapUtils.h"
#include "translationLayer.h"
#include "actions.h"
#include "root.h"

bool sloppyFocus;
int borderPx;
float borderColor[4];
float focusColor[4];
float overlayColor[4];
float textColor[4];
float selOverlayColor[4];
float selTextColor[4];

struct wlr_list tagNames;
struct rule rules[MAXLEN];

//MonitorRule monrules[MAXLEN];

int repeatRate;
int repeatDelay;
int innerGap;
int outerGap;

struct wlr_list tagNames;
char *termcmd;
Key *keys = NULL;
Key *buttons = NULL;

void updateConfig(lua_State *L)
{
    loadConfig(L, "config.lua");
    sloppyFocus = getConfigBool(L, "sloppyFocus");
    borderPx = getConfigInt(L, "borderPx");

    /* gaps */
    innerGap = getConfigInt(L, "innerGap");
    outerGap = getConfigInt(L, "outerGap");
    configureGaps(&innerGap, &outerGap);

    /* appearance */
    getConfigFloatArr(L, root.color, "rootColor");
    getConfigFloatArr(L, borderColor, "borderColor");
    getConfigFloatArr(L, focusColor, "focusColor");
    getConfigFloatArr(L, overlayColor, "overlayColor");
    getConfigFloatArr(L, textColor, "textColor");
    getConfigFloatArr(L, selOverlayColor, "overlayColor");
    getConfigFloatArr(L, selTextColor, "textColor");

    wlr_list_init(&tagNames);
    getConfigStrArr(L, &tagNames, "tagNames");
    getConfigRuleArr(L, rules, "rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt(L, "repeatRate");
    repeatDelay = getConfigInt(L, "repeatDelay");
    defaultLayout = getConfigLayout(L, "defaultLayout");
    prevLayout = (struct layout){.symbol = "", .funcId = 0};

    /* commands */
    termcmd = getConfigStr(L, "termcmd");
    getConfigKeyArr(L, keys, "keys");
    getConfigKeyArr(L, buttons, "buttons");
}

int reloadConfig(lua_State *L)
{
    printf("reload\n");
    updateConfig(L);

    // reconfigure clients
    struct client *c = NULL;
    wl_list_for_each(c, &clients, link) {
        c->bw = borderPx;
    }

    lua_pushboolean(L, true);
    arrangeThis(L);
    return 0;
}
