#include "parseConfig.h"
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/util/log.h>
#include <wordexp.h>
#include <libgen.h>
#include "tile/tileUtils.h"
#include "utils/gapUtils.h"
#include "translationLayer.h"
#include "actions.h"
#include "root.h"

const char *config_paths[] = {
    "$HOME/.config/juliawm/",
    "$XDG_CONFIG_HOME/juliawm/",
    "/etc/juliawm/",
};

bool sloppyFocus;
int borderPx;
float borderColor[4];
float focusColor[4];
float overlayColor[4];
float textColor[4];
float selOverlayColor[4];
float selTextColor[4];

struct wlr_list tagNames;
struct rule rules[MAXLEN];

//MonitorRule monrules[MAXLEN];

int repeatRate;
int repeatDelay;
int innerGap;
int outerGap;

struct wlr_list tagNames;
char *termcmd;
Key *keys = NULL;
Key *buttons = NULL;

static bool file_exists(const char *path) {
    return path && access(path, R_OK) != -1;
}

char *get_config_layout()
{
    for (size_t i = 0; i < sizeof(config_paths) / sizeof(char *); ++i) {
        wordexp_t p;
        if (wordexp(config_paths[i], &p, WRDE_UNDEF) == 0) {
            char *path = strdup(p.we_wordv[0]);
            wordfree(&p);
            if (file_exists(path)) {
                return path;
            }
            free(path);
        }
    }

    return NULL;
}

char *get_config_path()
{
    for (size_t i = 0; i < sizeof(config_paths) / sizeof(char *); ++i) {
        wordexp_t p;
        if (wordexp(config_paths[i], &p, WRDE_UNDEF) == 0) {
            char *path = strdup(p.we_wordv[0]);
            wordfree(&p);
            if (file_exists(path)) {
                return path;
            }
            free(path);
        }
    }

    return NULL;
}

void append_to_path(lua_State *L, const char *path)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    const char * curr_path = luaL_checkstring(L, -1);
    lua_pop(L, 1);
    char *path_var = calloc(1,
            strlen(curr_path) + 1 + strlen(path) + strlen("/?.lua"));

    strcpy(path_var, curr_path);
    strcat(path_var, ";");
    strcat(path_var, path);
    join_path(path_var, "/?.lua");
    lua_pushstring(L, path_var);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
    free(path_var);
}

int update_config(lua_State *L)
{
    // init
    char *config_path = get_config_path();
    append_to_path(L, config_path);

    if (load_config(L, config_path)) {
        wlr_log(WLR_ERROR, "file didn't load correctly");
        return 1;
    }
    free(config_path);

    sloppyFocus = get_config_bool(L, "sloppyFocus");
    borderPx = get_config_int(L, "borderPx");

    /* gaps */
    innerGap = get_config_int(L, "innerGap");
    outerGap = get_config_int(L, "outerGap");
    configure_gaps(&innerGap, &outerGap);

    /* appearance */
    get_config_float_arr(L, root.color, "rootColor");
    get_config_float_arr(L, borderColor, "borderColor");
    get_config_float_arr(L, focusColor, "focusColor");
    get_config_float_arr(L, overlayColor, "overlayColor");
    get_config_float_arr(L, textColor, "textColor");
    get_config_float_arr(L, selOverlayColor, "overlayColor");
    get_config_float_arr(L, selTextColor, "textColor");

    wlr_list_init(&tagNames);
    get_config_str_arr(L, &tagNames, "tagNames");
    get_config_rule_arr(L, rules, "rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = get_config_int(L, "repeatRate");
    repeatDelay = get_config_int(L, "repeatDelay");
    defaultLayout = getConfigLayout(L, "defaultLayout");
    prevLayout = (struct layout){.symbol = "", .funcId = 0};

    /* commands */
    termcmd = get_config_str(L, "termcmd");
    get_config_key_arr(L, keys, "keys");
    get_config_key_arr(L, buttons, "buttons");
    return 0;
}

int reloadConfig(lua_State *L)
{
    for (int i = 0; i < tagNames.length; i++)
        free(wlr_list_pop(&tagNames));
    wlr_list_finish(&tagNames);

    unsigned int focusedTag = selMon->tagset->focusedTag;
    unsigned int selTags = selMon->tagset->selTags[0];
    tagsetDestroy(selMon->tagset);
    update_config(L);
    selMon->tagset = tagsetCreate(&tagNames, focusedTag, selTags);

    // reconfigure clients
    struct client *c = NULL;
    wl_list_for_each(c, &clients, link) {
        c->bw = borderPx;
    }

    lua_pushboolean(L, true);
    arrangeThis(L);
    return 0;
}
