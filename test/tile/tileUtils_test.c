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

    clients[0].ws_id = 0;
    wlr_list_push(&ws0->visible_containers, &cons[0]);
    clients[1].ws_id = 0;
    wlr_list_push(&ws0->visible_containers, &cons[1]);
    clients[2].ws_id = 1;
    wlr_list_push(&ws1->visible_containers, &cons[2]);
    ck_assert_int_eq(get_container_count(ws0), 2);
} END_TEST

START_TEST(update_container_stack_positions_test)
{
    struct layout lt;
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    create_workspaces(&server.workspaces, tag_names, lt);

    struct monitor m0, m1;
    m0.ws_ids[0] = 0;
    m1.ws_ids[0] = 1;

    struct workspace *ws0 = get_workspace(&server.workspaces, 0);
    ws0->m = &m0;

    struct workspace *ws1 = get_workspace(&server.workspaces, 1);
    ws1->m = &m1;

    const int n = 4;
    struct client clients[n];
    struct container cons[n];
    for (int i = 0; i < n; i++) {
        cons[i].client = &clients[i];
        cons[i].position = INVALID_POSITION;
        cons[i].hidden = false;
    }

    clients[0].type = LAYER_SHELL;
    clients[0].ws_id = 0;
    clients[1].type = XDG_SHELL;
    clients[1].ws_id = 0;
    clients[2].type = XDG_SHELL;
    clients[2].ws_id = 0;
    clients[3].type = XDG_SHELL;
    clients[3].ws_id = 0;

    cons[0].floating = false;
    cons[0].m = &m0;
    wlr_list_push(&ws0->visible_containers, &cons[0]);

    cons[1].floating = false;
    cons[1].m = &m0;
    wlr_list_push(&ws0->visible_containers, &cons[1]);

    cons[2].floating = false;
    cons[2].m = &m1;
    wlr_list_push(&ws1->visible_containers, &cons[2]);

    cons[3].floating = true;
    cons[3].m = &m0;
    wlr_list_push(&ws0->floating_containers, &cons[3]);

    struct layout *lt_ptr = get_layout_in_monitor(&m0);
    lt_ptr->options.arrange_by_focus = false;

    ws0->n_all = get_container_count(ws0);
    ws1->n_all = get_container_count(ws1);

    update_container_stack_positions(&m0);
    ck_assert_int_eq(cons[0].position, INVALID_POSITION);
    ck_assert_int_ne(cons[1].position, INVALID_POSITION);
    ck_assert_int_eq(cons[2].position, INVALID_POSITION);
    ck_assert_int_ne(cons[3].position, INVALID_POSITION);

    lt_ptr->options.arrange_by_focus = true;
    update_container_stack_positions(&m0);
    ck_assert_int_eq(cons[0].position, INVALID_POSITION);
    ck_assert_int_ne(cons[1].position, INVALID_POSITION);
    ck_assert_int_eq(cons[2].position, INVALID_POSITION);
    ck_assert_int_ne(cons[3].position, INVALID_POSITION);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("tagset");
    tc = tcase_create("core");

    tcase_add_test(tc, get_container_count_test);
    tcase_add_test(tc, update_container_stack_positions_test);
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
