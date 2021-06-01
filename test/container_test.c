#include <check.h>

#include "monitor.h"
#include "container.h"
#include "workspace.h"
#include "utils/coreUtils.h"
#include "server.h"

START_TEST(test_visible_on)
{
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    wlr_list_push(&tag_names, "3");
    wlr_list_push(&tag_names, "4");

    struct layout lt;
    lt.options.arrange_by_focus = false;
    struct wlr_list workspaces;
    create_workspaces(&workspaces, &tag_names, &lt);

    struct monitor m0;
    struct monitor m1;

    struct workspace *ws0 = workspaces.items[0];
    struct workspace *ws1 = workspaces.items[1];
    ws0->m = &m0;
    ws1->m = &m1;

    struct client c;
    struct container *con = create_container(&c, &m0, false);

    con->m = &m0;
    con->client->ws_id = 0;
    con->hidden = false;
    ck_assert_int_eq(visible_on(con, ws0), true);

    con->m = &m1;
    con->client->ws_id = ws0->id;
    con->hidden = false;
    ck_assert_int_eq(visible_on(con, ws0), false);

    con->m = &m0;
    con->client->ws_id = ws1->id;
    con->hidden = false;
    ck_assert_int_eq(visible_on(con, ws1), false);

    con->m = &m1;
    con->client->ws_id = ws1->id;
    con->hidden = true;
    ck_assert_int_eq(visible_on(con, ws1), false);
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
    lt.options.arrange_by_focus = false;
    struct wlr_list workspaces;
    create_workspaces(&workspaces, &tag_names, &lt);

    struct monitor m0;
    struct monitor m1;

    struct workspace *ws0 = workspaces.items[0];
    struct workspace *ws1 = workspaces.items[1];

    ws0->m = &m0;
    ws1->m = &m1;

    struct client c;
    struct container *con = create_container(&c, &m0, false);

    con->m = &m0;
    con->client->ws_id = ws0->id;
    con->hidden = true;
    ck_assert_int_eq(exist_on(con, ws0), true);

    con->m = &m1;
    con->client->ws_id = ws0->id;
    con->hidden = false;
    ck_assert_int_eq(exist_on(con, ws0), false);

    con->m = &m0;
    con->client->ws_id = ws1->id;
    con->hidden = false;
    ck_assert_int_eq(exist_on(con, ws1), false);

    con->m = &m1;
    con->client->ws_id = ws1->id;
    con->hidden = false;
    ck_assert_int_eq(exist_on(con, ws1), true);
} END_TEST

START_TEST(focus_on_hidden_stack_test)
{
    // TODO fix this unittest
} END_TEST

START_TEST(focus_container_test)
{
} END_TEST

START_TEST(get_position_in_container_stack_crash_test)
{
    get_position_in_container_stack(NULL);
} END_TEST

START_TEST(get_focused_container_crash_test)
{
    get_focused_container(NULL);

    struct monitor m;
    m.ws_id = -1;
    get_focused_container(&m);

    m.ws_id = 700;
    get_focused_container(&m);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("container");
    tc = tcase_create("core");

    tcase_add_test(tc, test_visible_on);
    tcase_add_test(tc, test_exist_on);
    tcase_add_test(tc, focus_on_hidden_stack_test);
    tcase_add_test(tc, focus_container_test);
    tcase_add_test(tc, get_position_in_container_stack_crash_test);
    tcase_add_test(tc, get_focused_container_crash_test);
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
