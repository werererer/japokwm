#include <check.h>

#include "tagset.h"

START_TEST(flagToTagPositionTest)
{
    ck_assert_int_eq(TAG_ONE, positionToFlag(1));
    ck_assert_int_eq(TAG_EIGHT, positionToFlag(8));
} END_TEST

START_TEST(tagPositionToFlagTest)
{
    // on error this function returns TAG_ONE
    ck_assert_int_eq(1, flagToPosition(35));
    ck_assert_int_eq(1, flagToPosition(TAG_ONE));
    ck_assert_int_eq(8, flagToPosition(TAG_EIGHT));
} END_TEST

START_TEST(selLayoutTest)
{
    struct tagset tagset;

    // tagsetCreate depends on defaultLayout
    defaultLayout.symbol = "t";
    defaultLayout.arrange = NULL;

    tagsetCreate(&tagset);
    ck_assert_str_eq(tagset.lt[TAG_ONE].symbol, selLayout(&tagset).symbol);
    tagsetDestroy(&tagset);
} END_TEST

START_TEST(tagsOverlapTest)
{
    ck_assert_int_eq(tagsOverlap(2, 3), true);
    ck_assert_int_eq(tagsOverlap(6, 8), false);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tagset");
    tc = tcase_create("Core");

    tcase_add_test(tc, flagToTagPositionTest);
    tcase_add_test(tc, tagPositionToFlagTest);
    tcase_add_test(tc, tagPositionToFlagTest);
    tcase_add_test(tc, tagsOverlapTest);
    tcase_add_test(tc, selLayoutTest);
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
