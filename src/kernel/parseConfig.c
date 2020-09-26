#include "parseConfig.h"
#include <parseConfigUtils.h>

char* configFile = "../config.jl";

int sloppyfocus;
int borderpx;
float rootcolor[4];
float bordercolor[4];
float focuscolor[4];

char *tags[MAXLEN];
Rule rules[MAXLEN];

Layout layouts[MAXLEN];
MonitorRule monrules[MAXLEN];

int repeatRate;
int repeatDelay;

char *termcmd;
Hotkey keys[MAXLEN];
Hotkey buttons[MAXLEN];

void updateConfig()
{
    initConfig(configFile);
    sloppyfocus = getConfigInt("sloppyfocus");
    borderpx = getConfigInt("borderpx");

    /* appearance */
    getConfigFloatArr(rootcolor, "rootcolor");
    getConfigFloatArr(bordercolor, "bordercolor");
    getConfigFloatArr(focuscolor, "focuscolor");

    /* tagging */
    getConfigStrArr(tags, "tags");
    getConfigRuleArr(rules, "rules");
    getConfigLayoutArr(layouts, "layouts");

    /* monitors */
    getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt("repeatRate");
    repeatDelay = getConfigInt("repeatDelay");

    /* commands */
    getConfigStrArr(&termcmd, "termcmd");
    getConfigHotkeyArr(keys, "keys");
    // getConfigHotkeyArr(buttons, "buttons");
}
