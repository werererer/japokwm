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

void has_patrly_matching_keybinding_test()
{
    GPtrArray *registered_key_combos = g_ptr_array_new();

    GPtrArray *keycombos = g_ptr_array_new();
    struct keybinding i[] = {
        {.binding = "mod-S-Tab"},
        {.binding = "mod-1 mod-S-Tab"},
        {.binding = "mod-S-Tab mod-3"},
        {.binding = "mod-S-Tab mod-S-Tab"},
        {.binding = "mod-S-z mod-S-Tab"},
    };
    g_ptr_array_add(keycombos, &i[0]);
    g_ptr_array_add(keycombos, &i[1]);
    g_ptr_array_add(keycombos, &i[2]);
    g_ptr_array_add(keycombos, &i[3]);
    g_ptr_array_add(keycombos, &i[4]);

    bool b;

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "Alt_L-S-Tab");
    b = has_partly_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    g_ptr_array_add(registered_key_combos, "mod-S-1");
    b = has_partly_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-1");
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    b = has_partly_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    b = has_partly_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, true);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    b = has_partly_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-Tab-S");
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    // keybindings are required to be preprocessed (sorted etc)
    b = has_partly_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    g_ptr_array_free(registered_key_combos, TRUE);
    g_ptr_array_free(keycombos, TRUE);
}

void get_matching_keybinding_test()
{
    GPtrArray *registered_key_combos = g_ptr_array_new();

    GPtrArray *keycombos = g_ptr_array_new();
    struct keybinding i = {
        .binding = "mod-S-Tab mod-S-Tab",
    };
    g_ptr_array_add(keycombos, &i);

    bool b;

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "Alt_L-S-Tab");
    b = get_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    g_ptr_array_add(registered_key_combos, "mod-S-1");
    b = get_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-1");
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    b = get_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    b = get_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, true);

    list_clear(registered_key_combos, NULL);
    g_ptr_array_add(registered_key_combos, "mod-Tab-S");
    g_ptr_array_add(registered_key_combos, "mod-S-Tab");
    // keybindings are required to be preprocessed (sorted etc)
    b = get_matching_keybinding(keycombos, registered_key_combos);
    g_assert_cmpint(b, ==, false);

    g_ptr_array_free(registered_key_combos, TRUE);
    g_ptr_array_free(keycombos, TRUE);
}

void sort_keybinding_element_test()
{
    const char *element = "";
    char *res = NULL;
    struct options options = {
        .modkey = 0,
    };

    element = "mod-S-1";
    res = sort_keybinding_element(&options, element);
    g_assert_cmpstr(res, ==, "S-mod-1");
    free(res);

    element = "1-S-mod";
    res = sort_keybinding_element(&options, element);
    g_assert_cmpstr(res, ==, "S-mod-1");
    free(res);

    element = "S-mod-1";
    res = sort_keybinding_element(&options, element);
    g_assert_cmpstr(res, ==, "S-mod-1");
    free(res);

    element = "S-mod-C";
    res = sort_keybinding_element(&options, element);
    g_assert_cmpstr(res, ==, "C-S-mod");
    free(res);
}

void sort_keybinding_test()
{
    const char *element = "";
    char *res = NULL;

    struct options options = {
        .modkey = 0,
    };

    element = "mod-S-1 S-mod-2";
    res = sort_keybinding(&options, element);
    g_assert_cmpstr(res, ==, "S-mod-1 S-mod-2");
    free(res);

    element = "1-S-mod 2-mod-S";
    res = sort_keybinding(&options, element);
    g_assert_cmpstr(res, ==, "S-mod-1 S-mod-2");
    free(res);

    element = "S-mod-1 mod-2-S";
    res = sort_keybinding(&options, element);
    g_assert_cmpstr(res, ==, "S-mod-1 S-mod-2");
    free(res);
}

#define PREFIX "keybinding"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(has_patrly_matching_keybinding_test);
    add_test(get_matching_keybinding_test);
    add_test(sort_keybinding_element_test);
    add_test(sort_keybinding_test);

    return g_test_run();
}
