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

struct tagset tagset;
struct rule rules[MAXLEN];
struct layout defaultLayout;
struct layout prevLayout;

//MonitorRule monrules[MAXLEN];

int repeatRate;
int repeatDelay;

char *termcmd;
Key keys[MAXLEN];
Key buttons[MAXLEN];

void updateConfig()
{
    // create tagset since it is required see tagset.h
    tagsetCreate(&tagset);

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

    getConfigStrArr(tagset.tagNames, "tagNames");
    getConfigRuleArr(rules,"rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt("repeatRate");
    repeatDelay = getConfigInt("repeatDelay");
    defaultLayout = getConfigLayout("layout");
    prevLayout = (struct layout){.symbol = "", .arrange = NULL};

    /* commands */
    termcmd = getConfigStr("termcmd");
    getConfigKeyArr(keys, "keys");
    getConfigKeyArr(buttons, "buttons");
    // getConfigkeyArr(buttons, "buttons");
}
