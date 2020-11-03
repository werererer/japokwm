#include "parseConfig.h"
#include <julia.h>
#include <stdlib.h>
#include <string.h>
#include "utils/gapUtils.h"

char *mainModule = "juliawm";
char *configModule = "juliawm.config";
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
int innerGap;
int outerGap;

char *tagNames[MAXLEN];
char *termcmd;
Key keys[MAXLEN];
Key buttons[MAXLEN];

void updateConfig()
{
    char *module = configModule;
    sloppyFocus = getConfigInt(module, "sloppyFocus");
    borderPx = getConfigInt(module, "borderPx");

    /* gaps */
    innerGap = getConfigInt(module, "innerGap");
    outerGap = getConfigInt(module, "outerGap");
    configureGaps(&innerGap, &outerGap);

    /* appearance */
    getConfigFloatArr(module, rootColor, "rootColor");
    getConfigFloatArr(module, borderColor, "borderColor");
    getConfigFloatArr(module, focusColor, "focusColor");
    getConfigFloatArr(module, overlayColor, "overlayColor");
    getConfigFloatArr(module, textColor, "textColor");
    getConfigFloatArr(module, selOverlayColor, "overlayColor");
    getConfigFloatArr(module, selTextColor, "textColor");

    getConfigStrArr(module, tagNames, "tagNames");
    getConfigRuleArr(module, rules,"rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt(module, "repeatRate");
    repeatDelay = getConfigInt(module, "repeatDelay");
    defaultLayout = getConfigLayout(module, "defaultLayout");
    prevLayout = (struct layout){.symbol = "", .arrange = NULL};

    /* commands */
    termcmd = getConfigStr(module, "termcmd");
    getConfigKeyArr(module, keys, "keys");
    getConfigKeyArr(module, buttons, "buttons");
    // getConfigkeyArr(buttons, "buttons");
}
