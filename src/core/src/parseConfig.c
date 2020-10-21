#include "parseConfig.h"
#include "utils/parseConfigUtils.h"
#include <julia.h>

int sloppyFocus;
int borderPx;
float rootColor[4];
float borderColor[4];
float focusColor[4];
float overlayColor[4];
float textColor[4];
float selOverlayColor[4];
float selTextColor[4];

char *tags[MAXLEN];
Rule rules[MAXLEN];
Layout defaultLayout;
Layout prevLayout;

//MonitorRule monrules[MAXLEN];

int repeatRate;
int repeatDelay;

char *termcmd;
Key keys[MAXLEN];
Key buttons[MAXLEN];

void updateConfig()
{
    sloppyFocus = getConfigInt("sloppyFocus");
    borderPx = getConfigInt("borderPx");

    /* appearance */
    getConfigFloatArr(rootColor, "rootColor");
    getConfigFloatArr(borderColor, "borderColor");
    getConfigFloatArr(focusColor, "focusColor");
    getConfigFloatArr(overlayColor, "overlayColor");
    getConfigFloatArr(textColor, "textColor");
    getConfigFloatArr(selOverlayColor, "overlayColor");
    getConfigFloatArr(selTextColor, "textColor");

    /* tagging */
    getConfigStrArr(tags, "tags");
    getConfigRuleArr(rules, "rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt("repeatRate");
    repeatDelay = getConfigInt("repeatDelay");
    defaultLayout = getConfigLayout("layout");
    prevLayout = (Layout){.symbol = "", .arrange = NULL};

    /* commands */
    termcmd = getConfigStr("termcmd");
    getConfigKeyArr(keys, "keys");
    getConfigKeyArr(buttons, "buttons");
    // getConfigkeyArr(buttons, "buttons");
}
