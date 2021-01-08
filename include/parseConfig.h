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
extern char *main_module;
extern char *config_module;
extern bool sloppy_focus;
extern int border_px;
extern int inner_gap;
extern int outer_gap;
extern float border_color[4];
extern float root_color[4];
extern float focus_color[4];
extern float overlay_color[4];
extern float text_color[4];
extern float sel_overlay_color[4];
extern float sel_text_color[4];

extern struct rule rules[MAXLEN];
extern struct mon_rule monrules[MAXLEN];

extern int repeat_rate;
extern int repeat_delay;

/* commands */
extern struct wlr_list tag_names;
extern char *termcmd;
extern Key *keys;
extern Key *buttons;

/* sets global variables but needs  */
int update_config(lua_State *L);
int reload_config(lua_State *L);
void init_config_paths();
#endif /* PARSE_CONFIG_H */
