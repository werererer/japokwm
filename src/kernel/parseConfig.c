#include "parseConfig.h"

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
    getConfigInt(&sloppyfocus, "sloppyfocus");
    getConfigInt(&borderpx, "borderpx");

    /* appearance */
    getConfigFloatArr(rootcolor, "rootcolor");
    getConfigFloatArr(bordercolor, "bordercolor");
    getConfigFloatArr(focuscolor, "focuscolor");

    /* tagging */
    getConfigStrArr(tags, "tags");
    getConfigRules(rules, "rules");
    getConfigLayouts(layouts, "layouts");

    /* monitors */
    getConfigMonRules(monrules, "monrules");

    /* keyboard */
    getConfigIntArr(&repeatRate, "repeatRate");
    getConfigIntArr(&repeatDelay, "repeatDelay");

    /* commands */
    getConfigStrArr(&termcmd, "termcmd");
    getConfigHotkeys(keys, "keys");
    getConfigHotkeys(buttons, "buttons");
}
