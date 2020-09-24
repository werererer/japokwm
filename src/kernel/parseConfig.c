#include "config.h"

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
    getConfigHotKeys(keys, "keys");
    getConfigHotKeys(buttons, "buttons");
}
