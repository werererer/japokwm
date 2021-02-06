#include "keybinding.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"
#include <string.h>

const char *mods[8] = {"Shift_L", "Caps_Lock", "Control_L", "Alt_L", "", "", "Super_L", "ISO_Level3_Shift"};
const char *modkeys[4] = {"Alt_L", "Num_Lock", "ISO_Level3_Shift", "Super_L"};
const char *mouse[3] = {"Pointer_Button1", "Pointer_Button2", "Pointer_Button3"};

static size_t modifiers;
/*
 * convert mod to mask
 */
inline static unsigned int mod_to_mask(unsigned int x)
{
    return 1 << x;
}

static void mod_to_string(char *dest, unsigned int mod)
{
    for (int i = 0; i < 7; i++) {
        modifiers = mod;
        if ((mod & mod_to_mask(i)) != 0) {
            strcat(dest, mods[i]);
            strcat(dest, "-");
        }
    }
}

static void sym_to_binding(char *dest, int mods, int sym)
{
    mod_to_string(dest, mods);
    strcat(dest, XKeysymToString(sym));
}

// this function converts a string to a xkeysym string element
static void resolve_keybind_element(char *sym_dest, const char *bind)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = m->ws[0];
    struct layout *lt = &ws->layout[0];

    if (strcmp(bind, "mod") == 0) {
        strcpy(sym_dest, modkeys[lt->options.modkey]);
        return;
    }
    if (strcmp(bind, "M1") == 0) {
        strcpy(sym_dest, mouse[0]);
        return;
    }
    if (strcmp(bind, "M2") == 0) {
        strcpy(sym_dest, mouse[1]);
        return;
    }
    if (strcmp(bind, "M3") == 0) {
        strcpy(sym_dest, mouse[2]);
        return;
    }
    if (strcmp(bind, "S") == 0) {
        strcpy(sym_dest, "Shift_L");
        return;
    }
    strcpy(sym_dest, bind);
}

static bool is_same_keybind_element(const char *bind, const char *bind2)
{
    struct wlr_list bindarr = split_string(bind, "-");
    struct wlr_list bind2arr = split_string(bind2, "-");

    if (bind2arr.length == 0)
        return true;
    if (bindarr.length != bind2arr.length)
        return false;

    // remove all resolved items out of bind2arr found in bindarr
    for (int i = 0; i < bindarr.length; i++) {
        int str1len = strlen(bind2arr.items[i]);
        char bindelem[str1len];
        resolve_keybind_element(bindelem, bindarr.items[i]);

        for (int j = 0; j < bind2arr.length; j++) {
            int str2len = strlen(bind2arr.items[j]);
            char bind2elem[str2len];
            resolve_keybind_element(bind2elem, bind2arr.items[j]);

            if (strcmp(bindarr.items[i], bind2elem) == 0) {
                wlr_list_del(&bind2arr, j);
                break;
            }
        }
    }

    // if no items remain in bind2arr the bind must be correct
    bool ret = bind2arr.length == 0;

    return ret;
}

static bool is_same_keybind(const char *bind, const char *bind2)
{
    bool same = false;
    same = is_same_keybind_element(bind, bind2);
    return same;
}

static bool process_binding(lua_State *L, char *bind, int lua_ref)
{
    bool handled = false;
    lua_rawgeti(L, LUA_REGISTRYINDEX, lua_ref);
    int len = lua_rawlen(L, -1);
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -1, i+1);
        lua_rawgeti(L, -1, 1);
        const char *ref = luaL_checkstring(L, -1);
        lua_pop(L, 1);
        if (is_same_keybind(bind, ref)) {
            lua_rawgeti(L, -1, 2);
            lua_call_safe(L, 0, 0, 0);
            handled = true;
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    return handled;
}

bool button_pressed(int mods, int sym)
{
    // TODO make this safer
    char bind[128] = "";
    sym_to_binding(bind, mods, sym);
    bool handled = process_binding(L, bind, server.options.buttonbindings_ref);
    return handled;
}

bool key_pressed(int mods, int sym)
{
    char bind[128] = "";
    sym_to_binding(bind, mods, sym);
    bool handled = process_binding(L, bind, server.options.keybinds_ref);
    return handled;
}

bool key_state_has_modifiers(size_t mods)
{
    return modifiers & mods;
}
