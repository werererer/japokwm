#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"

/*
 * This file uses parseconfig to parse the config files
*/

/* appearance */
extern char *mainModule;
extern char *configModule;
extern bool sloppyFocus;
extern int borderPx;
extern int innerGap;
extern int outerGap;
extern float rootColor[4];
extern float borderColor[4];
extern float focusColor[4];
extern float overlayColor[4];
extern float textColor[4];
extern float selOverlayColor[4];
extern float selTextColor[4];

extern struct rule rules[MAXLEN];

extern int repeatRate;
extern int repeatDelay;

/* commands */
extern char **tagNames;
extern char *termcmd;
extern Key *keys;
extern Key *buttons;

/* sets global variables but needs  */
void updateConfig(lua_State *L);
int reloadConfig(lua_State *L);
#endif /* PARSE_CONFIG_H */
