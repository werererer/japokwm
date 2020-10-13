#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "parseConfigUtils.h"

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
extern char *tags[MAXLEN];
extern Rule rules[MAXLEN];

extern Layout defaultLayout;
extern Layout prevLayout;
//extern MonitorRule monrules[MAXLEN];

extern int repeatRate;
extern int repeatDelay;

/* commands */
extern char *termcmd;
extern Key keys[MAXLEN];
extern Key buttons[MAXLEN];

void updateConfig();
void updateLayout();
#endif /* PARSE_CONFIG_H */
