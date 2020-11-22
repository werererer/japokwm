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

extern const char *config_paths[];

/* appearance */
extern char *mainModule;
extern char *configModule;
extern bool sloppyFocus;
extern int borderPx;
extern int innerGap;
extern int outerGap;
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
extern struct wlr_list tagNames;
extern char *termcmd;
extern Key *keys;
extern Key *buttons;

/* returned char pointer must be freed */
char *get_config_layout_path();
/* returned char pointer must be freed */
char *get_config_file(const char *file);
/* returned char pointer must be freed */
char *get_config_dir(const char *file);

/* sets global variables but needs  */
int update_config(lua_State *L);
int reloadConfig(lua_State *L);
void init_config_paths();
#endif /* PARSE_CONFIG_H */
