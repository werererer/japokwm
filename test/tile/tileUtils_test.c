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
    create_workspaces(&server.workspaces, tag_names, lt);

    struct monitor m0, m1;
    struct workspace *ws0 = get_workspace(&server.workspaces, 0);
    ws0->m = &m0;
    struct workspace *ws1 = get_workspace(&server.workspaces, 1);
    ws1->m = &m1;

    wl_list_init(&containers);

    const int container_count = 3;
    struct client clients[container_count];
    for (int i = 0; i < container_count; i++) {
        clients[0].type = XDG_SHELL;
        clients[0].sticky = false;
    }
    clients[0].ws_id = 1;
    clients[1].ws_id = 0;
    clients[2].ws_id = 1;

    struct container cons[container_count];
    for (int i = 0; i < container_count; i++) {
        cons[i].client = &clients[i];
        cons[i].floating = false;
        cons[i].m = &m0;
        wl_list_insert(&containers, &cons[i].mlink);
    }

    ck_assert_int_eq(get_container_count(ws0), 2);
} END_TEST

START_TEST(get_tiled_container_count_test)
{
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    struct layout lt;
    create_workspaces(&server.workspaces, tag_names, lt);

    struct monitor m0, m1;
    struct workspace *ws0 = get_workspace(&server.workspaces, 0);
    ws0->m = &m0;
    struct workspace *ws1 = get_workspace(&server.workspaces, 1);
    ws1->m = &m1;

    wl_list_init(&containers);

    const int container_count = 3;
    struct client clients[container_count];
    for (int i = 0; i < container_count; i++) {
        clients[0].type = XDG_SHELL;
        clients[0].sticky = false;
    }
    clients[0].ws_id = 0;
    clients[1].ws_id = 0;
    clients[2].ws_id = 1;

    struct container cons[3];
    cons[0].client = &clients[0];
    cons[0].floating = false;
    cons[0].m = &m0;
    wl_list_insert(&containers, &cons[0].mlink);
    cons[1].client = &clients[1];
    cons[1].floating = false;
    cons[1].m = &m0;
    wl_list_insert(&containers, &cons[1].mlink);
    cons[2].client = &clients[2];
    cons[2].floating = false;
    cons[2].m = &m0;
    wl_list_insert(&containers, &cons[2].mlink);

    lt.options.arrange_by_focus = true;
    ck_assert_int_eq(get_tiled_container_count(ws0), 3);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tagset");
    tc = tcase_create("core");

    tcase_add_test(tc, get_container_count_test);
    tcase_add_test(tc, get_tiled_container_count_test);
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
