#include <stdlib.h>
#include <glib.h>
#include <wlr/util/box.h>
#include <wlr/util/edges.h>

#include "utils/coreUtils.h"
#include "utils/gapUtils.h"

void containerAddGapsTest()
{
    struct wlr_box con;
    con.x = 50;
    con.y = 50;
    con.width = 100;
    con.height = 100;

    struct wlr_box con2 = con;
    int gap = 5;

    container_add_gaps(&con2, gap, WLR_EDGE_LEFT | WLR_EDGE_RIGHT);
    g_assert_cmpint(con2.x, ==, 55);
    g_assert_cmpint(con2.y, ==,50);
    g_assert_cmpint(con2.width, ==, 90);
    g_assert_cmpint(con2.height, ==, 100);

    con2 = con;

    container_add_gaps(&con2, gap, WLR_EDGE_TOP);
    g_assert_cmpint(con2.x, ==, 50);
    g_assert_cmpint(con2.y, ==, 55);
    g_assert_cmpint(con2.width, ==, 100);
    g_assert_cmpint(con2.height, ==, 95);

    con2 = con;

    container_add_gaps(&con2, gap, WLR_EDGE_BOTTOM);
    g_assert_cmpint(con2.x, ==, 50);
    g_assert_cmpint(con2.y, ==, 50);
    g_assert_cmpint(con2.width, ==, 100);
    g_assert_cmpint(con2.height, ==, 95);
}

void containerSurroundGapsTest()
{
    struct wlr_box con;
    con.x = 50;
    con.y = 50;
    con.width = 100;
    con.height = 100;
    container_surround_gaps(&con, 4.0);
    g_assert_cmpint(con.x, ==, 52);
    g_assert_cmpint(con.y, ==, 52);
    g_assert_cmpint(con.width, ==, 96);
    g_assert_cmpint(con.height, ==, 96);
}

#define PREFIX "gapUtils"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(containerAddGapsTest);
    add_test(containerSurroundGapsTest);

    return g_test_run();
}
