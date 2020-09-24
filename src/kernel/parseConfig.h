#ifndef CONFIG_H
#define CONFIG_H

#include "parseconfig.h"

/*
 * This file uses parseconfig to parse the config files
*/

/* appearance */
extern int sloppyfocus;
extern int borderpx;
extern float rootcolor[4];
extern float bordercolor[4];
extern float focuscolor[4];

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
