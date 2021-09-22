#include <stdlib.h>
#include <glib.h>

#include "server.h"
#include "keybinding.h"

void test_keybinding()
{
    /* server.registered_key_combos = g_ptr_array_new(); */
    /* g_ptr_array_add(server.registered_key_combos, "hi"); */
    /* debug_print("glength: %i\n", server.registered_key_combos->len); */
    /* bool same = is_old_combo_same("hi"); */
    /* debug_print("return\n"); */
    /* g_assert_cmpint(same, ==, 1); */
    /* show_scratchpad(); */
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
