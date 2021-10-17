#ifndef KEYBINDING_H
#define KEYBINDING_H
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

// modifiers
#define MOD_SHIFT (1 << 0)
#define MOD_CAPS_LOCK (1 << 1)
#define MOD_CONTROL_L (1 << 2)
#define MOD_ALT_L (1 << 3)
// TODO find out what (1 << 4) and (1 << 5) are
#define MOD_SUPER_L (1 << 6)
#define MOD_ISO_LEVEL3_Shift (1 << 7)

struct layout;
struct options;

struct keybinding {
    char *binding;
    int lua_func_ref;
};

struct keybinding *create_keybinding(const char *binding, int lua_func_ref);
void destroy_keybinding(struct keybinding *keybinding);
void destroy_keybinding0(void *keybinding);

void *copy_keybinding(const void *keybinding_ptr, void *user_data);

char *sort_keybinding_element(struct options *options, const char *binding_element);
char *sort_keybinding(struct options *options, const char *binding);

bool has_partly_matching_keybinding(
        GPtrArray *keybindings,
        GPtrArray *registered_key_combos);
struct keybinding *get_matching_keybinding(
        GPtrArray *keybindings,
        GPtrArray *registered_key_combos);
bool has_keybind_same_amount_of_elements(GPtrArray *registered_key_combos, const char *bind);

int cmp_keybinding(const void *keybinding1, const void *keybinding2);

char *mod_to_keybinding(int mods, int sym);
// WARNING: This function will change the state of the windowmanager that means
// that certain things will be freed so don't trust your local variables that
// were assigned before calling this function anymore. Global variables should
// be fine. They might contain a different value after calling this function
// thou.
bool handle_keyboard_key(const char *bind);

bool key_state_has_modifiers(size_t mods);
#endif /* KEYBINDING_H */
