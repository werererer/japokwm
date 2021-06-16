#include <check.h>
#include <stdio.h>

#include "workspace.h"

START_TEST(workspace_contains_client_crash_test)
{
    /* workspace_contains_client(NULL, NULL); */
} END_TEST

START_TEST(workspace_has_clients_test)
{
    /* struct workspace *ws = create_workspace("test", 3); */
    /* ck_assert_int_eq(workspace_has_clients(ws), false); */
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("workspace");
    tc = tcase_create("core");

    tcase_add_test(tc, workspace_has_clients_test);
    tcase_add_test(tc, workspace_contains_client_crash_test);
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
