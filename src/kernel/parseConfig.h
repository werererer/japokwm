#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "parseConfigUtils.h"

/*
 * This file uses parseconfig to parse the config files
*/

extern char* configFile;

/* appearance */
extern int sloppyfocus;
extern int borderpx;
extern float rootcolor[4];
extern float bordercolor[4];
extern float focuscolor[4];

#define MAXLEN 15
extern char *tags[MAXLEN];
extern Rule rules[MAXLEN];

extern Layout layouts[MAXLEN];
extern MonitorRule monrules[MAXLEN];

extern int repeat_rate;
extern int repeat_delay;

/* commands */
extern char *termcmd;
extern Hotkey keys[MAXLEN];
extern Hotkey buttons[MAXLEN];

void updateConfig();
#endif /* PARSE_CONFIG_H */
