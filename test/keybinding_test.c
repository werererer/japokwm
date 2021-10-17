#include "server.h"
#include "keybinding.h"

#include <stdlib.h>
#include <glib.h>

struct test_keybinding {
    const char *binding;
    bool is_correct;
};

void test_keybinding()
{
    GPtrArray *registered_key_combos = g_ptr_array_new();
    g_ptr_array_add(registered_key_combos, "mod-S-1");

    const char *bind = "";
    bool b;

    bind = "mod-S-1";
    b = has_keybind_same_existing_elements(registered_key_combos, bind);
    g_assert_cmpint(b, ==, false);

    bind = "mod-S-Tab";
    b = has_keybind_same_existing_elements(registered_key_combos, bind);
    g_assert_cmpint(b, ==, true);
}

#define PREFIX "keybinding"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(test_keybinding);

    return g_test_run();
}
