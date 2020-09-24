#include "parseconfig.h"

/* appearance */
static const int sloppyfocus;  /* focus follows mouse */
static const unsigned int borderpx;  /* border pixel of windows */
static const float rootcolor[];
static const float bordercolor[];
static const float focuscolor[];

/* tagging */
static const char *tags[];
static const Rule rules[];
/* layout(s) */
static const Layout layouts[];
/* monitors */
static const MonitorRule monrules[];
/* keyboard */
static const struct xkb_rule_names xkb_rules;
static const int repeat_rate = 25;
static const int repeat_delay = 600;

/* commands */
static const char *termcmd;
static const Key keys[];
static const Button buttons[];

//TODO: int array
#define LOAD(X) (_Generic((X), \
    int: getConfig_int("X", &X),\
    float: getConfig_float("X", &X),\
    unsigned int: (unsigned)getConfig_int("X", &X),\
    char **: getConfig_str("X", X),\
    default: other))

void updateConfig()
{
    LOAD(sloppyfocus);
    LOAD(borderpx);

    /* appearance */
    LOAD(rootcolor);
    LOAD(bordercolor);
    LOAD(focuscolor);

    /* tagging */
    LOAD(tags);
    LOAD(rules);
    LOAD(layouts);

    /* monitors */
    LOAD(monrules);

    /* keyboard */
    LOAD(xkb_rules);
    LOAD(repeat_rate);
    LOAD(repeat_delay);

    /* commands */
    LOAD(termcmd);
    LOAD(keys);
    LOAD(buttons);
}
