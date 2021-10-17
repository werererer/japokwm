#include "server.h"
#include "keybinding.h"
#include "layout.h"
#include "options.h"

#include <stdlib.h>
#include <glib.h>
#include <lua.h>
#include <lauxlib.h>

struct test_keybinding {
    const char *binding;
    bool is_correct;
};

void has_keybind_same_existing_elements_test()
{
    GPtrArray *registered_key_combos = g_ptr_array_new();
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");

    const char *bind = "";
    bool b;

    struct layout lt = {
        .options = create_options(),
    };

    bind = "mod-S-1";
    b = has_keybind_same_existing_elements(&lt, registered_key_combos, bind);
    g_assert_cmpint(b, ==, false);

    bind = "mod-S-Tab";
    b = has_keybind_same_existing_elements(&lt, registered_key_combos, bind);
    g_assert_cmpint(b, ==, true);

    destroy_options(lt.options);
}

#define PREFIX "keybinding"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(has_keybind_same_existing_elements_test);

    return g_test_run();
}
