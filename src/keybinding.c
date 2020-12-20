#include "keybinding.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"
#include <string.h>

static size_t modifiers;
/*
 * convert mod to mask
 */
inline static unsigned int modMask(unsigned int x)
{
    return 1 << x;
}

static void modToString(char *res, unsigned int mod)
{
    lua_getglobal(L, "mods");
    for (int i = 0; i < 7; i++) {
        modifiers = mod;
        if ((mod & modMask(i)) != 0) {
            lua_rawgeti(L, -1, i+1);
            strcat(res, luaL_checkstring(L, -1));
            strcat(res, " ");
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

static void symToBinding(char *res, int mods, int sym)
{
    modToString(res, mods);
    strcat(res, XKeysymToString(sym));
}

static bool is_same_keybind(const char *bind, const char *bind2)
{
    struct wlr_list bindarr = split_string(bind, " ");
    struct wlr_list bind2arr = split_string(bind2, " ");

    if (bind2arr.length == 0)
        return false;
    if (bindarr.length != bind2arr.length)
        return false;

    // remove all items out of bind2arr found in bindarr
    for (int i = 0; i < bindarr.length; i++) {
        for (int j = 0; j < bind2arr.length; j++) {
            if (strcmp(bindarr.items[i], bind2arr.items[j]) == 0) {
                wlr_list_del(&bind2arr, j);
                break;
            }
        }
    }
    // if no items remain in bind2arr the bind must be correct
    bool ret = bind2arr.length == 0;

    return ret;
}

static bool process_binding(char *bind, const char *reference)
{
    bool handled = false;
    lua_getglobal(L, reference);
    int len = lua_rawlen(L, -1);
    for (int i = 1; i <= len; i++) {
        lua_rawgeti(L, -1, i);
        lua_rawgeti(L, -1, 1);
        const char *ref = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        if (is_same_keybind(bind, ref)) {
            lua_rawgeti(L, -1, 2);
            lua_pushinteger(L, selected_layout(selected_monitor)->containers_info.n);
            lua_pcall(L, 1, 0, 0);
            handled = true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return handled;
}

bool button_pressed(int mods, int sym)
{
    char bind[128] = "";
    symToBinding(bind, mods, sym);
    bool handled = process_binding(bind, "buttons");
    return handled;
}

bool key_pressed(int mods, int sym)
{
    char bind[128] = "";
    symToBinding(bind, mods, sym);
    bool handled = process_binding(bind, "keys");
    return handled;
}

bool key_state_has_modifiers(size_t mods)
{
    return modifiers & mods;
}
