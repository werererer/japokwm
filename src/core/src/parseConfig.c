#include "parseConfig.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include "utils/gapUtils.h"

bool sloppyFocus;
int borderPx;
float rootColor[4];
float borderColor[4];
float focusColor[4];
float overlayColor[4];
float textColor[4];
float selOverlayColor[4];
float selTextColor[4];

struct rule rules[MAXLEN];

//MonitorRule monrules[MAXLEN];

int repeatRate;
int repeatDelay;
int innerGap;
int outerGap;

char **tagNames;
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
    getConfigFloatArr(L, rootColor, "rootColor");
    getConfigFloatArr(L, borderColor, "borderColor");
    getConfigFloatArr(L, focusColor, "focusColor");
    getConfigFloatArr(L, overlayColor, "overlayColor");
    getConfigFloatArr(L, textColor, "textColor");
    getConfigFloatArr(L, selOverlayColor, "overlayColor");
    getConfigFloatArr(L, selTextColor, "textColor");

    getConfigStrArr(L, tagNames, "tagNames");
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
