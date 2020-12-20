#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "utils/coreUtils.h"
#include "utils/parseConfigUtils.h"

/*
 * This file uses parseconfig to parse the config files
*/

struct config {
    char *file;
};

/* appearance */
extern char *mainModule;
extern char *configModule;
extern bool sloppyFocus;
extern int borderPx;
extern int inner_gap;
extern int outer_gap;
extern float borderColor[4];
extern float rootColor[4];
extern float focusColor[4];
extern float overlayColor[4];
extern float textColor[4];
extern float selOverlayColor[4];
extern float selTextColor[4];

extern struct rule rules[MAXLEN];

extern int repeat_rate;
extern int repeat_delay;

/* commands */
extern struct wlr_list tag_names;
extern char *termcmd;
extern Key *keys;
extern Key *buttons;

/* sets global variables but needs  */
int update_config(lua_State *L);
int reloadConfig(lua_State *L);
void init_config_paths();
#endif /* PARSE_CONFIG_H */
