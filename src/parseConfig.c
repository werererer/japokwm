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
    "$HOME/.juliawm/layout/",
    "$XDG_CONFIG_HOME/juliawm/layout/",
    SYSCONFDIR "/juliawm/layout/",
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

void init_config_paths()
{
    char *config_home = getenv("XDG_CONFIG_HOME");
    if (!config_home || !*config_home) {
        config_paths[1] = "$HOME/.config/juliawm/layout/";
    }
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
    strcat(path_var, "/?.lua");
    lua_pushstring(L, path_var);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);
    free(path_var);
}

int update_config(lua_State *L)
{
    // init
    char *config_path = get_config_path();
    printf("current Path %s\n", config_path);
    init_config_paths();
    append_to_path(L, "");
    if (loadConfig(L, config_path)) {
        wlr_log(WLR_ERROR, "file didn't load correctly");
        return 1;
    }

    sloppyFocus = getConfigBool(L, "sloppyFocus");
    borderPx = getConfigInt(L, "borderPx");

    /* gaps */
    innerGap = getConfigInt(L, "innerGap");
    outerGap = getConfigInt(L, "outerGap");
    configureGaps(&innerGap, &outerGap);

    /* appearance */
    getConfigFloatArr(L, root.color, "rootColor");
    getConfigFloatArr(L, borderColor, "borderColor");
    getConfigFloatArr(L, focusColor, "focusColor");
    getConfigFloatArr(L, overlayColor, "overlayColor");
    getConfigFloatArr(L, textColor, "textColor");
    getConfigFloatArr(L, selOverlayColor, "overlayColor");
    getConfigFloatArr(L, selTextColor, "textColor");

    wlr_list_init(&tagNames);
    getConfigStrArr(L, &tagNames, "tagNames");
    getConfigRuleArr(L, rules, "rules");

    /* monitors */
    //getConfigMonRuleArr(monrules, "monrules");

    /* keyboard */
    repeatRate = getConfigInt(L, "repeatRate");
    repeatDelay = getConfigInt(L, "repeatDelay");
    defaultLayout = getConfigLayout(L, "defaultLayout");
    prevLayout = (struct layout){.symbol = "", .funcId = 0};

    /* commands */
    termcmd = getConfigStr(L, "termcmd");
    getConfigKeyArr(L, keys, "keys");
    getConfigKeyArr(L, buttons, "buttons");
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
