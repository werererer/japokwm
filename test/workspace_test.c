#include <check.h>
#include <stdio.h>

#include "workspace.h"

START_TEST(push_workspace_crash_test)
{
    /* push_workspace(NULL, NULL); */
} END_TEST

START_TEST(reset_loaded_layouts_test)
{
    struct wlr_list tag_names;
    wlr_list_init(&tag_names);
    wlr_list_push(&tag_names, "0");
    wlr_list_push(&tag_names, "1");
    wlr_list_push(&tag_names, "2");
    wlr_list_push(&tag_names, "3");

    lua_State *L = luaL_newstate();
    struct layout *lt0 = create_layout(L);
    struct layout *lt1 = create_layout(L);
    struct layout *lt2 = create_layout(L);
    struct wlr_list workspaces;
    create_workspaces(&workspaces, &tag_names, lt0);

    struct workspace *ws0 = workspaces.items[0];
    wlr_list_push(&ws0->loaded_layouts, lt0);
    wlr_list_push(&ws0->loaded_layouts, lt1);
    wlr_list_push(&ws0->loaded_layouts, lt2);

    reset_loaded_layouts(&workspaces);
    ck_assert_int_eq(ws0->loaded_layouts.length, 0);
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("workspace");
    tc = tcase_create("core");

    tcase_add_test(tc, push_workspace_crash_test);
    tcase_add_test(tc, reset_loaded_layouts_test);
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
