#include "parseConfig.h"

int sloppyfocus;
int borderpx;
float rootcolor[4];
float bordercolor[4];
float focuscolor[4];

char *tags[MAXLEN];
Rule rules[MAXLEN];

Layout layouts[MAXLEN];
MonitorRule monrules[MAXLEN];

int repeat_rate;
int repeat_delay;

char *termcmd;
Hotkey keys[MAXLEN];
Hotkey buttons[MAXLEN];

void updateConfig()
{
    initConfig("../config.jl");
    getConfig_int(&sloppyfocus, "sloppyfocus");
    getConfig_int(&borderpx, "borderpx");

    /* appearance */
    getConfigArr_float(rootcolor, "rootcolor");
    getConfigArr_float(bordercolor, "bordercolor");
    getConfigArr_float(focuscolor, "focuscolor");

    //TODO: style?
    /* tagging */
    getConfigArr_str(tags, "tags");
    getConfigRules(rules, "rules");
    getConfigLayouts(layouts, "layouts");

    /* monitors */
    getConfigMonRules(monrules, "monrules");

    /* keyboard */
    getConfigArr_int(&repeat_rate, "repeat_rate");
    getConfigArr_int(&repeat_delay, "repeat_delay");

    /* commands */
    getConfigArr_str(&termcmd, "termcmd");
    getConfigHotkeys(keys, "keys");
    getConfigHotkeys(buttons, "buttons");
}
