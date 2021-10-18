#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "utils/coreUtils.h"
#include "stringop.h"

void join_string_test()
{
    char *res = NULL;

    {
        const char *g[] = {"1", "2", "3"};
        res = join_string(g, LENGTH(g), ";");
        g_assert_cmpstr(res, ==, "1;2;3");
        free(res);
    }
    {
        const char *g[] = {"one", "two", "three", "four"};
        res = join_string(g, LENGTH(g), "XXX");
        g_assert_cmpstr(res, ==, "oneXXXtwoXXXthreeXXXfour");
        free(res);
    }
}

#define PREFIX "keybinding"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(join_string_test);

    return g_test_run();
}
