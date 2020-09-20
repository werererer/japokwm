#ifndef CONFIG
#define CONFIG

#include "parseconfig.h"

/* appearance */
int sloppyfocus;
int borderpx;
float rootcolor[4];
float bordercolor[4];
float focuscolor[4];

#define MAXLEN 15
/* tagging */
char *tags[MAXLEN];
Rule rules[MAXLEN];
/* layout(s) */
Layout layouts[MAXLEN];
/* monitors */
MonitorRule monrules[MAXLEN];
/* keyboard */
int repeat_rate = 25;
int repeat_delay = 600;

/* commands */
char *termcmd;
Hotkey keys[MAXLEN] = {};
Hotkey buttons[MAXLEN] = {};

void updateConfig();
#endif
