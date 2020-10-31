#include "parseConfig.h"
#include <julia.h>
#include <stdlib.h>
#include <string.h>

int sloppyFocus;
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
float innerGap;
float outerGap;

char *tagNames[MAXLEN];
char *termcmd;
Key keys[MAXLEN];
Key buttons[MAXLEN];

void updateConfig()
{
    sloppyFocus = getConfigInt("sloppyFocus");
    borderPx = getConfigInt("borderPx");
    innerGap = getConfigFloat("innerGap");
    outerGap = getConfigFloat("outerGap");

    /* appearance */
    getConfigFloatArr(rootColor, "rootColor");
    getConfigFloatArr(borderColor, "borderColor");
    getConfigFloatArr(focusColor, "focusColor");
    getConfigFloatArr(overlayColor, "overlayColor");
    getConfigFloatArr(textColor, "textColor");
    getConfigFloatArr(selOverlayColor, "overlayColor");
    getConfigFloatArr(selTextColor, "textColor");

    getConfigStrArr(tagNames, "tagNames");
    getConfigRuleArr(rules,"rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt("repeatRate");
    repeatDelay = getConfigInt("repeatDelay");
    defaultLayout = getConfigLayout("defaultLayout");
    prevLayout = (struct layout){.symbol = "", .arrange = NULL};

    /* commands */
    termcmd = getConfigStr("termcmd");
    getConfigKeyArr(keys, "keys");
    getConfigKeyArr(buttons, "buttons");
    // getConfigkeyArr(buttons, "buttons");
}
