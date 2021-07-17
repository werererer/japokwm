#include <check.h>
#include <stdlib.h>

#include "utils/coreUtils.h"

START_TEST(dir_exists_test)
{
    ck_assert_int_eq(dir_exists("/"), true);
    ck_assert_int_eq(dir_exists(""), false);
} END_TEST

START_TEST(get_on_composed_list_test)
{
    GPtrArray *lists;

    GPtrArray *list1;
    GPtrArray *list2;
    GPtrArray *list3;

    lists = g_ptr_array_new();

    list1 = g_ptr_array_new();
    list2 = g_ptr_array_new();
    list3 = g_ptr_array_new();

    g_ptr_array_add(lists, list1);
    g_ptr_array_add(lists, list2);
    g_ptr_array_add(lists, list3);

    g_ptr_array_add(list1, "0");
    g_ptr_array_add(list1, "1");

    g_ptr_array_add(list1, "2");
    g_ptr_array_add(list2, "3");
    g_ptr_array_add(list2, "4");
    g_ptr_array_add(list2, "5");

    g_ptr_array_add(list3, "6");
    g_ptr_array_add(list3, "7");
    g_ptr_array_add(list3, "8");

    ck_assert_str_eq(get_in_composed_list(lists, 0), "0");
    ck_assert_str_eq(get_in_composed_list(lists, 1), "1");
    ck_assert_str_eq(get_in_composed_list(lists, 2), "2");
    ck_assert_str_eq(get_in_composed_list(lists, 3), "3");
    ck_assert_str_eq(get_in_composed_list(lists, 4), "4");
    ck_assert_str_eq(get_in_composed_list(lists, 5), "5");
    ck_assert_str_eq(get_in_composed_list(lists, 6), "6");
    ck_assert_str_eq(get_in_composed_list(lists, 7), "7");
    ck_assert_str_eq(get_in_composed_list(lists, 8), "8");
} END_TEST

START_TEST(remove_from_composed_list_test)
{
    GPtrArray *lists;

    GPtrArray *list1;
    GPtrArray *list2;
    GPtrArray *list3;

    lists = g_ptr_array_new();

    list1 = g_ptr_array_new();
    list2 = g_ptr_array_new();
    list3 = g_ptr_array_new();

    g_ptr_array_add(lists, list1);
    g_ptr_array_add(lists, list2);
    g_ptr_array_add(lists, list3);

    g_ptr_array_add(list1, "0");
    g_ptr_array_add(list1, "1");

    g_ptr_array_add(list2, "2");
    g_ptr_array_add(list2, "3");
    g_ptr_array_add(list2, "4");
    g_ptr_array_add(list2, "5");

    g_ptr_array_add(list3, "6");
    g_ptr_array_add(list3, "7");
    g_ptr_array_add(list3, "8");
    g_ptr_array_add(list3, "9");

    delete_from_composed_list(lists, 0);
    ck_assert_str_eq(get_in_composed_list(lists, 0), "1");
    ck_assert_str_eq(get_in_composed_list(lists, 1), "2");
    g_ptr_array_insert(list1, 0, "0");

    delete_from_composed_list(lists, 1);
    ck_assert_str_eq(get_in_composed_list(lists, 0), "0");
    ck_assert_str_eq(get_in_composed_list(lists, 1), "2");
    g_ptr_array_insert(list1, 1, "1");

    delete_from_composed_list(lists, 2);
    ck_assert_str_eq(get_in_composed_list(lists, 1), "1");
    ck_assert_str_eq(get_in_composed_list(lists, 2), "3");
    g_ptr_array_insert(list2, 0, "2");

    delete_from_composed_list(lists, 3);
    ck_assert_str_eq(get_in_composed_list(lists, 2), "2");
    ck_assert_str_eq(get_in_composed_list(lists, 3), "4");
    g_ptr_array_insert(list2, 1, "3");

    delete_from_composed_list(lists, 7);
    ck_assert_str_eq(get_in_composed_list(lists, 6), "6");
    ck_assert_str_eq(get_in_composed_list(lists, 7), "8");
    g_ptr_array_insert(list3, 1, "7");

    delete_from_composed_list(lists, 8);
    ck_assert_str_eq(get_in_composed_list(lists, 7), "7");
    ck_assert_str_eq(get_in_composed_list(lists, 8), "9");
    g_ptr_array_insert(list3, 2, "8");
} END_TEST

START_TEST(wlr_list_remove_in_composed_list_test)
{
    GPtrArray *lists;

    GPtrArray *list1;
    GPtrArray *list2;
    GPtrArray *list3;

    lists = g_ptr_array_new();

    list1 = g_ptr_array_new();
    list2 = g_ptr_array_new();
    list3 = g_ptr_array_new();

    g_ptr_array_add(lists, list1);
    g_ptr_array_add(lists, list2);
    g_ptr_array_add(lists, list3);

    g_ptr_array_add(list1, "0");
    g_ptr_array_add(list1, "1");

    g_ptr_array_add(list1, "2");
    g_ptr_array_add(list2, "3");
    g_ptr_array_add(list2, "4");
    g_ptr_array_add(list2, "5");

    g_ptr_array_add(list3, "6");
    g_ptr_array_add(list3, "7");
    g_ptr_array_add(list3, "8");

    remove_in_composed_list(lists, (int (*)(const void *, const void *))strcmp, "1");
    ck_assert_str_eq(get_in_composed_list(lists, 1), "2");
} END_TEST

START_TEST(wlr_list_find_in_composed_list_test)
{
    GPtrArray *lists;

    GPtrArray *list1;
    GPtrArray *list2;
    GPtrArray *list3;

    lists = g_ptr_array_new();

    list1 = g_ptr_array_new();
    list2 = g_ptr_array_new();
    list3 = g_ptr_array_new();

    g_ptr_array_add(lists, list1);
    g_ptr_array_add(lists, list2);
    g_ptr_array_add(lists, list3);

    g_ptr_array_add(list1, "0");
    g_ptr_array_add(list1, "1");

    g_ptr_array_add(list2, "2");
    g_ptr_array_add(list2, "3");
    g_ptr_array_add(list2, "4");
    g_ptr_array_add(list2, "5");

    g_ptr_array_add(list3, "6");
    g_ptr_array_add(list3, "7");
    g_ptr_array_add(list3, "8");

    int position = find_in_composed_list(lists, cmp_str, "1");
    ck_assert_int_eq(position, 1);
} END_TEST

START_TEST(cross_sum_test)
{
    ck_assert_int_eq(cross_sum(0b111, 2), 3);
    ck_assert_int_eq(cross_sum(100, 10), 1);
} END_TEST

START_TEST(get_relative_item_in_list_test)
{
    GPtrArray *lists;

    GPtrArray *list1;
    GPtrArray *list2;
    GPtrArray *list3;

    lists = g_ptr_array_new();

    list1 = g_ptr_array_new();
    list2 = g_ptr_array_new();
    list3 = g_ptr_array_new();

    g_ptr_array_add(lists, list1);
    g_ptr_array_add(lists, list2);
    g_ptr_array_add(lists, list3);

    g_ptr_array_add(list1, "0");
    g_ptr_array_add(list1, "1");

    g_ptr_array_add(list2, "2");
    g_ptr_array_add(list2, "3");
    g_ptr_array_add(list2, "4");
    g_ptr_array_add(list2, "5");

    g_ptr_array_add(list3, "6");
    g_ptr_array_add(list3, "7");
    g_ptr_array_add(list3, "8");

    ck_assert_str_eq(get_relative_item_in_list(list1, 0, -1), "1");
    ck_assert_str_eq(get_relative_item_in_list(list2, 0, -1), "5");
    ck_assert_str_eq(get_relative_item_in_list(list3, 0, -1), "8");

    ck_assert_str_eq(get_relative_item_in_composed_list(lists, 0, 1), "1");
    ck_assert_str_eq(get_relative_item_in_composed_list(lists, 3, 4), "7");
    ck_assert_str_eq(get_relative_item_in_composed_list(lists, 3, -2), "1");
} END_TEST

Suite *suite()
{
    Suite *s;
    TCase *tc;

    s = suite_create("coreUtils");
    tc = tcase_create("core");

    tcase_add_test(tc, dir_exists_test);
    tcase_add_test(tc, get_on_composed_list_test);
    tcase_add_test(tc, remove_from_composed_list_test);
    tcase_add_test(tc, wlr_list_remove_in_composed_list_test);
    tcase_add_test(tc, wlr_list_find_in_composed_list_test);
    tcase_add_test(tc, cross_sum_test);
    tcase_add_test(tc, get_relative_item_in_list_test);
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
