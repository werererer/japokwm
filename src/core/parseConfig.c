#include "parseConfig.h"
#include "parseConfigUtils.h"
#include <julia.h>

int sloppyfocus;
int borderpx;
float rootcolor[4];
float bordercolor[4];
float focuscolor[4];

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
    sloppyfocus = getConfigInt("sloppyfocus");
    borderpx = getConfigInt("borderpx");

    /* appearance */
    getConfigFloatArr(rootcolor, "rootcolor");
    getConfigFloatArr(bordercolor, "bordercolor");
    getConfigFloatArr(focuscolor, "focuscolor");


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
