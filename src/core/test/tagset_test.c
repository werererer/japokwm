#include "tagset.h"
#include <check.h>

START_TEST(flagToTagPositionTest)
{
    enum tagPosition tp = 4;
    ck_assert_int_eq(TAG_FOUR, tagPositionToFlag(tp));
} END_TEST

START_TEST(tagPositionToFlagTest)
{
    ck_assert_int_eq(TAG_ONE, flagToTagPosition(49));
    ck_assert_int_eq(TAG_FOUR, flagToTagPosition(16));
} END_TEST


Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tagset");
    tc = tcase_create("Core");

    tcase_add_test(tc, flagToTagPositionTest);
    tcase_add_test(tc, tagPositionToFlagTest);
    suite_add_tcase(s, tc);

    return s;
}

int main()
{
    bool success = false;
    Suite *s;
    SRunner *runner;

    s = suite();
    runner = srunner_create(s);

    srunner_run_all(runner, CK_NORMAL);
    success = srunner_ntests_run(runner);
    srunner_free(runner);
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
