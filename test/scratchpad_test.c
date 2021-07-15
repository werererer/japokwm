#include <stdlib.h>
#include <check.h>

#include "scratchpad.h"

START_TEST(show_scratchpad_crash_test)
{
    /* show_scratchpad(); */
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("scratchpad");
    tc = tcase_create("Core");

    tcase_add_test(tc, show_scratchpad_crash_test);

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
