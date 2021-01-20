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
extern float text_color[4];
extern float sel_overlay_color[4];
extern float sel_text_color[4];

extern size_t rule_count;

/* commands */
extern struct layout *keys;
extern struct layout *buttons;

/* sets global variables but needs  */
int update_config(lua_State *L);
int reload_config(lua_State *L);
void init_config_paths();
#endif /* PARSE_CONFIG_H */
