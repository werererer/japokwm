#include <check.h>
#include <stdio.h>

#include "workspace.h"

START_TEST(push_workspace_crash_test)
{
    push_workspace(NULL, NULL);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("client");
    tc = tcase_create("core");

    tcase_add_test(tc, push_workspace_crash_test);
    suite_add_tcase(s, tc);

    return s;
}

int main()
{
    setbuf(stdout, NULL);

    int number_failed;
    Suite *s;
    SRunner *sr;

    s = suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    srunner_ntests_run(sr);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
