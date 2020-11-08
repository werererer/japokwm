#include "keybinding.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"

/*
 * convert mod to mask
 */
static unsigned int modMask(unsigned int x)
{
    return 1 << x;
}

static void modToString(char *res, unsigned int mod)
{
    lua_getglobal(L, "mods");
    for (int i = 0; i < 7; i++) {
        if ((mod & modMask(i)) != 0) {
            lua_rawgeti(L, -1, i+1);
            strcat(res, luaL_checkstring(L, -1));
            strcat(res, " ");
            lua_pop(L, 1);
        }
    }
}

static void symToBinding(char *res, int mods, int sym)
{
    modToString(res, mods);
    strcat(res, XKeysymToString(sym));
}

// TODO: unittest
static bool isSameKeybind(const char *bind, const char *bind2)
{
    bool sameKeybind = true;

    char *found;
    const char delim = ' ';
    char *str = strdup(bind);
    char *strPtr = str;
    char *str2 = strdup(bind2);
    // replace all tokens appearing in bind in bind2 with spaces
    while ( (found = strsep(&str, &delim)) != NULL ) {
        char *substring = strstr(str2, found);
        int len = strlen(found);
        found[len] = delim;
        if (substring) {
            memset(substring, delim, len);
        } else {
            sameKeybind = false;
            break;
        }
    }

    if (sameKeybind) {
        // does str2 only consist of spaces? no -> not sameKeybind
        for (int i = 0; i < strlen(str2); i++) {
            if (str2[i] != delim) {
                sameKeybind = false;
                break;
            }
        }
    }
    free(str2);
    free(strPtr);

    return sameKeybind;
}

static bool processBinding(char *bind, const char *reference)
{
    bool handled = false;
    printf("BIND: %s|\n", bind);
    lua_getglobal(L, reference);
    int len = lua_rawlen(L, -1);
    for (int i = 1; i <= len; i++) {
        lua_rawgeti(L, -1, i);
        lua_rawgeti(L, -1, 1);
        const char *s = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        if (isSameKeybind(bind, s)) {
            lua_rawgeti(L, -1, 2);
            lua_pushinteger(L, containersInfo.n);
            lua_pcall(L, 1, 0, 0);
            handled = true;
        }
        lua_pop(L, 1);
    }
    return handled;
}

bool buttonPressed(int mods, int sym)
{
    char *bind = calloc(128, sizeof(char));
    symToBinding(bind, mods, sym);
    bool handled = processBinding(bind, "buttons");
    free(bind);
    return handled;
}

bool keyPressed(int mods, int sym)
{
    char *bind = calloc(128, sizeof(char));
    symToBinding(bind, mods, sym);
    bool handled = processBinding(bind, "keys");
    free(bind);
    return handled;
}
