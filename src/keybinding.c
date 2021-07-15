#include "keybinding.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"

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
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = ws->layout;

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
    if (strcmp(bind, "C") == 0) {
        strcpy(sym_dest, "Control_L");
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
    GPtrArray *bindarr = split_string(bind, "-");
    GPtrArray *bind2arr = split_string(bind2, "-");

    if (bind2arr->len == 0)
        return true;
    if (bindarr->len != bind2arr->len)
        return false;

    // remove all resolved items out of bind2arr found in bindarr
    for (int i = 0; i < bindarr->len; i++) {
        int str1len = strlen(g_ptr_array_index(bind2arr, i));
        char bindelem[str1len];
        resolve_keybind_element(bindelem, g_ptr_array_index(bindarr, i));

        for (int j = 0; j < bind2arr->len; j++) {
            int str2len = strlen(g_ptr_array_index(bind2arr, j));
            char bind2elem[str2len];
            resolve_keybind_element(bind2elem, g_ptr_array_index(bind2arr, j));

            if (strcmp(g_ptr_array_index(bindarr, i), bind2elem) == 0) {
                g_ptr_array_remove_index(bind2arr, j);
                break;
            }
        }
    }

    // if no items remain in bind2arr the bind must be correct
    bool ret = bind2arr->len == 0;

    return ret;
}

static bool is_same_keybind(const char *bind, const char *bind2)
{
    bool same = is_same_keybind_element(bind, bind2);
    return same;
}

static bool process_binding(lua_State *L, char *bind, GPtrArray *keybindings)
{
    bool handled = false;
    for (int i = 0; i < keybindings->len; i++) {
        struct keybinding *keybinding = g_ptr_array_index(keybindings, i);
        if (is_same_keybind(bind, keybinding->binding)) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, keybinding->lua_func_ref);
            lua_call_safe(L, 0, 0, 0);
            handled = true;
        }
    }
    return handled;
}

struct keybinding *create_keybinding()
{
    struct keybinding *keybinding = calloc(1, sizeof(struct keybinding));
    return keybinding;
}

bool handle_keybinding(int mods, int sym)
{
    char bind[128] = "";
    sym_to_binding(bind, mods, sym);
    bool handled = process_binding(L, bind, server.default_layout->options.keybindings);
    return handled;
}

bool key_state_has_modifiers(size_t mods)
{
    return modifiers & mods;
}
