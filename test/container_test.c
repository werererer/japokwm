#include <check.h>

#include "monitor.h"
#include "container.h"
#include "workspace.h"

START_TEST(test_visible_on)
{
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    wlr_list_push(&tag_names, "3");
    wlr_list_push(&tag_names, "4");

    struct layout lt;
    struct wlr_list workspaces;
    create_workspaces(&workspaces, tag_names, lt);

    struct monitor m1;
    struct monitor m2;
    get_workspace(&workspaces, 0)->m = &m1;
    get_workspace(&workspaces, 1)->m = &m2;

    struct client c;
    struct container con = {
        .client = &c,
    };

    con.m = &m1;
    con.client->ws_id = 0;
    ck_assert_int_eq(visible_on(&con, &workspaces, 0), true);

    con.m = &m2;
    con.client->ws_id = 0;
    ck_assert_int_eq(visible_on(&con, &workspaces, 0), false);

    con.m = &m1;
    con.client->ws_id = 1;
    ck_assert_int_eq(visible_on(&con, &workspaces, 1), false);

    con.m = &m2;
    con.client->ws_id = 1;
    ck_assert_int_eq(visible_on(&con, &workspaces, 1), true);
} END_TEST

START_TEST(test_exist_on)
{
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    wlr_list_push(&tag_names, "3");
    wlr_list_push(&tag_names, "4");

    struct layout lt;
    struct wlr_list workspaces;
    create_workspaces(&workspaces, tag_names, lt);

    struct monitor m1;
    struct monitor m2;
    get_workspace(&workspaces, 0)->m = &m1;
    get_workspace(&workspaces, 1)->m = &m2;

    struct client c;
    struct container con = {
        .client = &c,
    };

    con.m = &m1;
    con.client->ws_id = 0;
    con.hidden = true;
    ck_assert_int_eq(exist_on(&con, &workspaces, 0), true);

    con.m = &m2;
    con.client->ws_id = 0;
    con.hidden = false;
    ck_assert_int_eq(exist_on(&con, &workspaces, 0), false);

    con.m = &m1;
    con.client->ws_id = 1;
    ck_assert_int_eq(exist_on(&con, &workspaces, 1), false);

    con.m = &m2;
    con.client->ws_id = 1;
    ck_assert_int_eq(exist_on(&con, &workspaces, 1), true);
} END_TEST

Suite *suite() {
    Suite *s;
    TCase *tc;

    s = suite_create("client");
    tc = tcase_create("core");

    tcase_add_test(tc, test_visible_on);
    tcase_add_test(tc, test_exist_on);
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
