#include <stdlib.h>
#include <check.h>
#include <wlr/types/wlr_box.h>
#include <wlr/util/edges.h>

#include "utils/coreUtils.h"
#include "utils/gapUtils.h"

START_TEST(containerAddGapsTest)
{
    Container con;
    con.x = 50;
    con.y = 50;
    con.width = 100;
    con.height = 100;

    Container con2 = con;
    int gap = 5;

    containerAddGaps(&con2, gap, WLR_EDGE_LEFT | WLR_EDGE_RIGHT);
    ck_assert_double_eq(con2.x, 55);
    ck_assert_double_eq(con2.y, 50);
    ck_assert_double_eq(con2.width, 95);
    ck_assert_double_eq(con2.height, 100);

    con2 = con;

    containerAddGaps(&con2, gap, WLR_EDGE_TOP);
    ck_assert_double_eq(con2.x, 50);
    ck_assert_double_eq(con2.y, 55);
    ck_assert_double_eq(con2.width, 100);
    ck_assert_double_eq(con2.height, 100);

    con2 = con;

    containerAddGaps(&con2, gap, WLR_EDGE_BOTTOM);
    ck_assert_double_eq(con2.x, 50);
    ck_assert_double_eq(con2.y, 50);
    ck_assert_double_eq(con2.width, 100);
    ck_assert_double_eq(con2.height, 95);
} END_TEST

START_TEST(containerSurroundGapsTest)
{
    Container con;
    con.x = 50;
    con.y = 50;
    con.width = 100;
    con.height = 100;
    containerSurroundGaps(&con, 4.0);
    ck_assert_double_eq(con.x, 52);
    ck_assert_double_eq(con.y, 52);
    ck_assert_double_eq(con.width, 96);
    ck_assert_double_eq(con.height, 96);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tagset");
    tc = tcase_create("Core");
    tcase_add_test(tc, containerAddGapsTest);
    tcase_add_test(tc, containerSurroundGapsTest);

    suite_add_tcase(s, tc);

    return s;
}

int main()
{
    int numberFailed;
    Suite *s;
    SRunner *sr;

    s = suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    srunner_ntests_run(sr);
    numberFailed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (numberFailed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
