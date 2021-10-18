#include "keybinding.h"

#include <string.h>
#include <X11/Xlib.h>

#include "input_manager.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "utils/parseConfigUtils.h"
#include "stringop.h"
#include "workspace.h"
#include "monitor.h"

const char *mods[8] = {"Shift_L", "Caps_Lock", "Control_L", "Alt_L", "", "", "Super_L", "ISO_Level3_Shift"};
const char *modkeys[4] = {"Alt_L", "Num_Lock", "ISO_Level3_Shift", "Super_L"};
const char *mouse[3] = {"Pointer_Button1", "Pointer_Button2", "Pointer_Button3"};

static bool sym_is_modifier(const char *sym);
static int cmp_keybinding_strings(const char *binding1, const char *binding2);
static size_t modifiers;
/*
 * convert mod to mask
 */
inline static unsigned int mod_to_mask(unsigned int x)
{
    return 1 << x;
}

static void mod_to_string(char **dest, unsigned int mod)
{
    for (int i = 0; i < LENGTH(mods); i++) {
        modifiers = mod;
        if ((mod & mod_to_mask(i)) != 0) {
            append_string(dest, mods[i]);
            append_string(dest, "-");
        }
    }
}

static void mod_to_binding(char **dest, int mods)
{
    mod_to_string(dest, mods);
}

static void sym_to_binding(char **dest, int sym)
{
    append_string(dest, XKeysymToString(sym));
}

// this function converts a string to a xkeysym string element
static char *resolve_keybind_element(struct options *options, const char *bind)
{
    if (strcmp(bind, "mod") == 0) {
        char *resolved = strdup(modkeys[options->modkey]);
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

static char *unresolve_keybind_element(struct options *options, const char *bind)
{
    if (strcmp(bind, modkeys[options->modkey]) == 0) {
        char *unresolved = strdup("mod");
        return unresolved;
    }
    if (strcmp(bind, mouse[0]) == 0) {
        char *unresolved = strdup("M1");
        return unresolved;
    }
    if (strcmp(bind, mouse[1]) == 0) {
        char *unresolved = strdup("M2");
        return unresolved;
    }
    if (strcmp(bind, mouse[2]) == 0) {
        char *unresolved = strdup("M3");
        return unresolved;
    }
    if (strcmp(bind, "Control_L") == 0) {
        char *unresolved = strdup("C");
        return unresolved;
    }
    if (strcmp(bind, "Shift_L") == 0) {
        char *unresolved = strdup("S");
        return unresolved;
    }
    char *unresolved = strdup(bind);
    return unresolved;
}

char *sort_keybinding_element(struct options *options, const char *binding_element)
{
    GPtrArray *bindarr = split_string(binding_element, "-");

    GPtrArray *mods = g_ptr_array_copy(bindarr, NULL, NULL);

    char *non_modifier = NULL;
    // most of the time the modifier is at the back so we loop backwards as an
    // optimization and there can only be one non modifier in a binding element
    // so we break instantly if found
    for (int i = mods->len-1; i >= 0; i--) {
        char *bind_atom = g_ptr_array_index(mods, i);

        char *resolved_bind_atom = resolve_keybind_element(options, bind_atom);
        bool is_modifier = sym_is_modifier(resolved_bind_atom);
        free(resolved_bind_atom);
        if (is_modifier) {
            continue;
        }

        non_modifier = strdup(bind_atom);
        g_ptr_array_remove_index_fast(mods, i);
        break;
    }

    g_ptr_array_sort(mods, cmp_strptr);

    if (non_modifier) {
        g_ptr_array_add(mods, non_modifier);
    }

    char *result = join_string((const char **)mods->pdata, mods->len, "-");

    if (non_modifier) {
        free(non_modifier);
    }

    g_ptr_array_free(mods, TRUE);
    g_ptr_array_set_free_func(bindarr, free);
    g_ptr_array_free(bindarr, TRUE);
    return result;
}

char *preprocess_binding(struct options *options, const char *binding)
{
    GPtrArray *bindarr = split_string(binding, "-");

    for (int i = 0; i < bindarr->len; i++) {
        char *bind_el = g_ptr_array_index(bindarr, i);
        char *res = unresolve_keybind_element(options, bind_el);
        free(bind_el);
        g_ptr_array_index(bindarr, i) = res;
    }

    char *res = join_string((const char **)bindarr->pdata, bindarr->len, "-");

    g_ptr_array_free(bindarr, true);
    return res;
}

char *sort_keybinding(struct options *options, const char *binding)
{
    GPtrArray *bindarr = split_string(binding, " ");
    g_ptr_array_set_free_func(bindarr, free);

    for (int i = 0; i < bindarr->len; i++) {
        char *bind_el = g_ptr_array_index(bindarr, i);
        char *res = sort_keybinding_element(options, bind_el);
        free(bind_el);
        g_ptr_array_index(bindarr, i) = res;
    }

    char *res = join_string((const char **)bindarr->pdata, bindarr->len, " ");

    g_ptr_array_free(bindarr, TRUE);

    return res;
}

static bool is_same_keybind_element(struct layout *lt, const char *bind, const char *bind2)
{
    bool ret_val = false;
    if (strcmp(bind, bind2) == 0) {
        return true;
    }

    return ret_val;
}

int cmp_partly_keybinding_strings(const char *binding1, const char *binding2)
{
    GPtrArray *k1_array = split_string(binding1, " ");
    g_ptr_array_set_free_func(k1_array, free);
    GPtrArray *k2_array = split_string(binding2, " ");
    g_ptr_array_set_free_func(k2_array, free);

    int ret_val = 0;
    int i1 = k1_array->len;
    int i2 = k2_array->len;
    if (i1 < i2) {
        for (int i = 0; i < k1_array->len; i++) {
            const char *el1 = g_ptr_array_index(k1_array, i);
            const char *el2 = g_ptr_array_index(k2_array, i);

            if (strcmp(el1, el2) != 0) {
                ret_val = -1;
                goto exit_return;
            }
        }
        ret_val = 0;
        goto exit_return;
    } else if ( i1 > i2 ) {
        ret_val = 1;
        goto exit_return;
    } else {
        ret_val = 1;
        goto exit_return;
    }

exit_return:
    g_ptr_array_unref(k2_array);
    g_ptr_array_unref(k1_array);

    return ret_val;
}

static int cmp_partly_keybinding_ptr_ptr(const void *key_ptr, const void *keybinding_ptr2)
{
    const char *key = key_ptr;
    const struct keybinding *k2 = *(const void **)keybinding_ptr2;

    int ret_val = cmp_partly_keybinding_strings(key, k2->binding);
    return ret_val;
}

bool has_partly_matching_keybinding(
        GPtrArray *keybindings,
        GPtrArray *registered_key_combos)
{
    char *binding = join_string((const char **)registered_key_combos->pdata, registered_key_combos->len, " ");

    struct keybinding **base = (struct keybinding **)keybindings->pdata;
    int len = keybindings->len;
    struct keybinding **keybinding = bsearch(
            binding,
            base,
            len,
            sizeof(struct keybinding *),
            cmp_partly_keybinding_ptr_ptr);
    free(binding);

    struct keybinding *ret_keybinding = NULL;
    if (keybinding) {
        ret_keybinding = *keybinding;
    }
    return ret_keybinding;
}

static int cmp_keybinding_ptr_ptr(const void *key_ptr, const void *keybinding_ptr2)
{
    const char *key = key_ptr;
    const struct keybinding *keybinding = *(const void **)keybinding_ptr2;
    int ret_value = cmp_keybinding_strings(key, keybinding->binding);
    return ret_value;
}

struct keybinding *get_matching_keybinding(
        GPtrArray *keybindings,
        GPtrArray *registered_key_combos)
{
    char *binding = join_string((const char **)registered_key_combos->pdata, registered_key_combos->len, " ");

    struct keybinding **base = (struct keybinding **)keybindings->pdata;
    int len = keybindings->len;
    struct keybinding **keybinding = bsearch(
            binding,
            base,
            len,
            sizeof(struct keybinding *),
            cmp_keybinding_ptr_ptr);
    free(binding);

    struct keybinding *ret_keybinding = NULL;
    if (keybinding) {
        ret_keybinding = *keybinding;
    }
    return ret_keybinding;
}

bool has_keybind_same_amount_of_elements(GPtrArray *registered_key_combos, const char *bind)
{
    GPtrArray *bind2combos = split_string(bind, " ");

    bool same_length = bind2combos->len == registered_key_combos->len;
    g_ptr_array_free(bind2combos, true);
    return same_length;
}

// WARNING: This function will change the state of the windowmanager that means that
// certain things will be freed so don't trust your local variables that were
// assigned before calling this function anymore. Global variables should be
// fine. They might contain a different value after calling this function thou.
static void execute_binding(lua_State *L, struct keybinding *keybinding)
{
    list_clear(server.registered_key_combos, free);
    lua_rawgeti(L, LUA_REGISTRYINDEX, keybinding->lua_func_ref);
    lua_call_safe(L, 0, 0, 0);
    server_allow_reloading_config();
}

struct keybinding *create_keybinding(const char *binding, int lua_func_ref)
{
    struct keybinding *keybinding = calloc(1, sizeof(*keybinding));
    keybinding->binding = strdup(binding);
    keybinding->lua_func_ref = lua_func_ref;
    return keybinding;
}

void destroy_keybinding(struct keybinding *keybinding)
{
    free(keybinding->binding);
    if (keybinding->lua_func_ref > 0) {
        luaL_unref(L, LUA_REGISTRYINDEX, keybinding->lua_func_ref);
    }
    free(keybinding);
}

void destroy_keybinding0(void *keybinding)
{
    destroy_keybinding(keybinding);
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
    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);
    long timeout = lt->options->key_combo_timeout;

    if (wl_event_source_timer_update(server.combo_timer_source, timeout) < 0) {
        printf("failed to disarm key repeat timer \n");
    }
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

void *copy_keybinding(const void *keybinding_ptr, void *user_data)
{
    const struct keybinding *keybinding = keybinding_ptr;
    lua_rawgeti(L, LUA_REGISTRYINDEX, keybinding->lua_func_ref);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    struct keybinding *kb_dup = create_keybinding(keybinding->binding, ref);
    return kb_dup;
}

int cmp_keybinding_strings(const char *binding1, const char *binding2)
{
    GPtrArray *k1_array = split_string(binding1, " ");
    g_ptr_array_set_free_func(k1_array, free);
    GPtrArray *k2_array = split_string(binding2, " ");
    g_ptr_array_set_free_func(k2_array, free);

    int ret_val = 0;
    int i1 = k1_array->len;
    int i2 = k2_array->len;
    if (i1 < i2) {
        ret_val = -1;
        goto exit_return;
    } else if ( i1 > i2 ) {
        ret_val = 1;
        goto exit_return;
    } else {
        ret_val = cmp_str(binding1, binding2);
        goto exit_return;
    }

exit_return:
    g_ptr_array_unref(k2_array);
    g_ptr_array_unref(k1_array);

    return ret_val;
}

int cmp_keybinding(const void *keybinding1, const void *keybinding2)
{
    const struct keybinding *k1 = *(void **)keybinding1;
    const struct keybinding *k2 = *(void **)keybinding2;

    int ret_val = cmp_keybinding_strings(k1->binding, k2->binding);
    return ret_val;
}

char *mod_to_keybinding(int mods, int sym)
{
    char *mod_bind = strdup("");
    mod_to_binding(&mod_bind, mods);
    char *sym_bind = strdup("");
    sym_to_binding(&sym_bind, sym);

    char *bind = g_strconcat(mod_bind, sym_bind, NULL);

    free(mod_bind);
    free(sym_bind);

    return bind;
}

bool process_binding(struct layout *lt, const char *bind)
{
    struct keybinding *keybinding =
        get_matching_keybinding(
                lt->options->keybindings,
                server.registered_key_combos
                );
    if (!keybinding) {
        list_clear(server.registered_key_combos, free);
        g_ptr_array_add(server.registered_key_combos, strdup(bind));

        keybinding =
            get_matching_keybinding(
                    lt->options->keybindings,
                    server.registered_key_combos
                    );
        // try again with no registered key combos
        if (!keybinding) {
            list_clear(server.registered_key_combos, free);
            return false;
        }
    }

    execute_binding(L, keybinding);
    return true;
}

bool handle_keyboard_key(const char *bind)
{
    reset_keycombo_timer(server.combo_timer_source);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct layout *lt = workspace_get_layout(ws);

    char *preprocessed_bind = preprocess_binding(lt->options, bind);
    char *sorted_bind = sort_keybinding_element(lt->options, preprocessed_bind);
    free(preprocessed_bind);

    g_ptr_array_add(server.registered_key_combos, strdup(sorted_bind));

    // DEBUG
    // for (int i = 0; i < lt->options->keybindings->len; i++) {
    //     struct keybinding *keybinding = g_ptr_array_index(lt->options->keybindings, i);
    //     printf("%s\n", keybinding->binding);
    // }
    //
    bool partly = has_partly_matching_keybinding(
                lt->options->keybindings,
                server.registered_key_combos
            );
    if (partly) {
        return true;
    }

    bool handled = process_binding(lt, sorted_bind);
    return handled;
}

bool key_state_has_modifiers(size_t mods)
{
    return modifiers & mods;
}
