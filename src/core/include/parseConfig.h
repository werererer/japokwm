#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "utils/parseConfigUtils.h"

/*
 * This file uses parseconfig to parse the config files
*/

/* appearance */
extern int sloppyFocus;
extern int borderPx;
extern float rootColor[4];
extern float borderColor[4];
extern float focusColor[4];
extern float overlayColor[4];
extern float textColor[4];
extern float selOverlayColor[4];
extern float selTextColor[4];

#define MAXLEN 15
extern char *tags[MAXLEN];
extern struct rule rules[MAXLEN];

extern Layout defaultLayout;
extern Layout prevLayout;

extern int repeatRate;
extern int repeatDelay;

/* commands */
extern char *termcmd;
extern Key keys[MAXLEN];
extern Key buttons[MAXLEN];

void updateConfig();
void updateLayout();
#endif /* PARSE_CONFIG_H */
