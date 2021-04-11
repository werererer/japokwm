#include <stdlib.h>
#include <check.h>

#include "server.h"
#include "tile/tileUtils.h"

START_TEST(get_container_count_test)
{
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    struct layout lt;
    create_workspaces(&server.workspaces, &tag_names, &lt);

    struct monitor m0, m1;
    struct workspace *ws0 = get_workspace(0);
    ws0->m = &m0;
    struct workspace *ws1 = get_workspace(1);
    ws1->m = &m1;

    const int container_count = 3;
    struct client clients[container_count];
    for (int i = 0; i < container_count; i++) {
        clients[0].type = XDG_SHELL;
        clients[0].sticky = false;
    }

    struct container cons[container_count];
    for (int i = 0; i < container_count; i++) {
        cons[i].client = &clients[i];
        cons[i].floating = false;
        cons[i].m = &m0;
    }

    clients[0].ws_selector.ws_id = 0;
    wlr_list_push(&ws0->tiled_containers, &cons[0]);
    clients[1].ws_selector.ws_id = 0;
    wlr_list_push(&ws0->tiled_containers, &cons[1]);
    clients[2].ws_selector.ws_id = 1;
    wlr_list_push(&ws1->tiled_containers, &cons[2]);
    ck_assert_int_eq(get_container_count(ws0), 2);
} END_TEST

START_TEST(get_relative_item_test)
{
    struct wlr_list lists;

    struct wlr_list list1;
    struct wlr_list list2;
    struct wlr_list list3;

    wlr_list_init(&lists);

    wlr_list_init(&list1);
    wlr_list_init(&list2);
    wlr_list_init(&list3);

    wlr_list_push(&lists, &list1);
    wlr_list_push(&lists, &list2);
    wlr_list_push(&lists, &list3);

    wlr_list_push(&list1, "0");
    wlr_list_push(&list1, "1");

    wlr_list_push(&list1, "2");
    wlr_list_push(&list2, "3");
    wlr_list_push(&list2, "4");
    wlr_list_push(&list2, "5");

    wlr_list_push(&list3, "6");
    wlr_list_push(&list3, "7");
    wlr_list_push(&list3, "8");

    ck_assert_str_eq(get_relative_item_in_composed_list(&lists, 4, 1), "5");
} END_TEST


Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tileUtils");
    tc = tcase_create("core");

    tcase_add_test(tc, get_container_count_test);
    tcase_add_test(tc, get_relative_item_test);
    suite_add_tcase(s, tc);

    return s;
}

int main()
{
    setbuf(stdout, NULL);
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
