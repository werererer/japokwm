#ifndef KEYBINDING_H
#define KEYBINDING_H
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// modifiers
#define MOD_SHIFT (1 << 0)
#define MOD_CAPS_LOCK (1 << 1)
#define MOD_CONTROL_L (1 << 2)
#define MOD_ALT_L (1 << 3)
// TODO find out what (1 << 4) and (1 << 5) are
#define MOD_SUPER_L (1 << 6)
#define MOD_ISO_LEVEL3_Shift (1 << 7)

struct keybinding {
    char *binding;
    int lua_func_ref;
};

struct keybinding *create_keybinding(const char *binding, int lua_func_ref);
void destroy_keybinding(struct keybinding *keybinding);
void destroy_keybinding0(void *keybinding);

bool handle_keybinding(int mod, int sym);

bool key_state_has_modifiers(size_t mods);
#endif /* KEYBINDING_H */
