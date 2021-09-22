#include "keybinding.h"

#include <string.h>
#include <X11/Xlib.h>

#include "input_manager.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"
#include "workspace.h"

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

static void mod_to_binding(char *dest, int mods)
{
    mod_to_string(dest, mods);
}

static void sym_to_binding(char *dest, int sym)
{
    strcpy(dest, XKeysymToString(sym));
}

// this function converts a string to a xkeysym string element
static char *resolve_keybind_element(const char *bind)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = ws->layout;

    if (strcmp(bind, "mod") == 0) {
        char *resolved = strdup(modkeys[lt->options.modkey]);
        return resolved;
    }
    if (strcmp(bind, "M1") == 0) {
        char *resolved = strdup(mouse[0]);
        return resolved;
    }
    if (strcmp(bind, "M2") == 0) {
        char *resolved = strdup(mouse[1]);
        return resolved;
    }
    if (strcmp(bind, "M3") == 0) {
        char *resolved = strdup(mouse[2]);
        return resolved;
    }
    if (strcmp(bind, "C") == 0) {
        char *resolved = strdup("Control_L");
        return resolved;
    }
    if (strcmp(bind, "S") == 0) {
        char *resolved = strdup("Shift_L");
        return resolved;
    }
    char *resolved = strdup(bind);
    return resolved;
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
        char *bind_el = g_ptr_array_index(bindarr, i);
        char *bindelem = resolve_keybind_element(bind_el);

        for (int j = 0; j < bind2arr->len; j++) {
            char *bind2_el = g_ptr_array_index(bind2arr, j);
            char *bind2elem = resolve_keybind_element(bind2_el);

            if (strcmp(bindelem, bind2elem) == 0) {
                g_ptr_array_remove_index(bind2arr, j);
                free(bind2elem);
                break;
            }
            free(bind2elem);
        }
        free(bindelem);
    }

    // if no items remain in bind2arr the bind must be correct
    bool ret = bind2arr->len == 0;

    return ret;
}

bool is_old_combo_same(const char *bind)
{
    GPtrArray *bind_combos = split_string(bind, " ");

    if (bind_combos->len < server.registered_key_combos->len) {
        return false;
    }

    bool is_same = false;
    for (int i = 0; i < server.registered_key_combos->len; i++) {
        char *server_bind = g_ptr_array_index(server.registered_key_combos, i);
        char *bind_combo = g_ptr_array_index(bind_combos, i);

        if (is_same_keybind_element(server_bind, bind_combo)) {
            is_same = true;
        } else {
            is_same = false;
            return false;
        }
    }
    return is_same;
}

bool is_same_keybind_completed(const char *bind, const char *bind2)
{
    GPtrArray *bind2combos = split_string(bind2, " ");
    if (bind2combos->len == server.registered_key_combos->len) {
        return true;
    }
    return false;
}

// bind2 is a saved bind
bool has_equal_keybind_element(const char *bind, GPtrArray *keybindings)
{
    int current_combo_index = server.registered_key_combos->len;
    for (int i = 0; i < keybindings->len; i++) {
        struct keybinding *keybinding = g_ptr_array_index(keybindings, i);
        const char *bind2 = keybinding->binding;
        GPtrArray *bind2_combos = split_string(bind2, " ");

        if (bind2_combos->len-1 < current_combo_index) {
            continue;
        }

        const char *_current_bind2_combo = g_ptr_array_index(bind2_combos, current_combo_index);
        char *current_bind2_combo = resolve_keybind_element(_current_bind2_combo);
        bool is_equal = is_same_keybind_element(bind, current_bind2_combo);
        free(current_bind2_combo);
        if (is_equal) {
            return true;
        }
    }
    return false;
}

static bool process_binding(lua_State *L, const char *bind, GPtrArray *keybindings)
{
    /* for (int i = 0; i < server.registered_key_combos->len; i++) { */
    /*     const char *comb = g_ptr_array_index(server.registered_key_combos, i); */
    /* } */
    bool handled = false;
    for (int i = 0; i < keybindings->len; i++) {
        struct keybinding *keybinding = g_ptr_array_index(keybindings, i);

        if (!is_old_combo_same(keybinding->binding))
            continue;
        if (!is_same_keybind_completed(bind, keybinding->binding))
            continue;

        handled = true;
        list_clear(server.registered_key_combos, free);
        lua_rawgeti(L, LUA_REGISTRYINDEX, keybinding->lua_func_ref);
        lua_call_safe(L, 0, 0, 0);
    }
    return handled;
}

struct keybinding *create_keybinding()
{
    struct keybinding *keybinding = calloc(1, sizeof(struct keybinding));
    return keybinding;
}

static int millisec_get_available_seconds(int milli_sec)
{
    int available_seconds = milli_sec/1000;
    return available_seconds;
}

static long millisec_to_nanosec(long milli_sec)
{
    return milli_sec * 100000;
}

static void reset_keycombo_timer(timer_t timer)
{
    long timeout = server.default_layout->options.key_combo_timeout;

    int available_seconds = millisec_get_available_seconds(timeout);
    timeout -= (1000*available_seconds);

    long nano_timeout = millisec_to_nanosec(timeout);

    struct itimerspec value = {
        .it_value = (struct timespec) {
            .tv_sec = available_seconds,
            .tv_nsec = nano_timeout,
        },
        .it_interval = (struct timespec) {
            .tv_sec = 0,
            .tv_nsec = 0,
        },
    };

    timer_settime(timer, 0, &value, NULL);
}

static bool sym_is_modifier(const char *sym)
{
    for (int i = 0; i < LENGTH(mods); i++) {
        const char *mod = mods[i];
        if (strcmp(mod, sym) == 0) {
            return true;
        }
    }
    return false;
}

bool handle_keybinding(int mods, int sym)
{
    reset_keycombo_timer(server.combo_timer);

    char bind[256] = "";
    char mod_bind[128] = "";
    mod_to_binding(mod_bind, mods);
    char sym_bind[128] = "";
    sym_to_binding(sym_bind, sym);

    strcpy(bind, mod_bind);
    strcat(bind, sym_bind);

    if (sym_is_modifier(sym_bind)) {
        return true;
    }

    if (!has_equal_keybind_element(bind, server.default_layout->options.keybindings)) {
        list_clear(server.registered_key_combos, free);

        // try again with no registered key combos
        if (!has_equal_keybind_element(bind, server.default_layout->options.keybindings)) {
            return false;
        }
    }
    g_ptr_array_add(server.registered_key_combos, strdup(bind));
    process_binding(L, bind, server.default_layout->options.keybindings);
    return true;
}

bool key_state_has_modifiers(size_t mods)
{
    return modifiers & mods;
}
