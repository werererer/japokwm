#include <glib.h>
#include <stdlib.h>

#include "glib.h"

#include "utils/coreUtils.h"

void dir_exists_test()
{
    g_assert_cmpint(dir_exists("/"), ==, true);
    g_assert_cmpint(dir_exists(""), ==, false);
}

void get_on_composed_list_test()
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

    g_assert_cmpstr(get_in_composed_list(lists, 0), ==, "0");
    g_assert_cmpstr(get_in_composed_list(lists, 1), ==, "1");
    g_assert_cmpstr(get_in_composed_list(lists, 2), ==, "2");
    g_assert_cmpstr(get_in_composed_list(lists, 3), ==, "3");
    g_assert_cmpstr(get_in_composed_list(lists, 4), ==, "4");
    g_assert_cmpstr(get_in_composed_list(lists, 5), ==, "5");
    g_assert_cmpstr(get_in_composed_list(lists, 6), ==, "6");
    g_assert_cmpstr(get_in_composed_list(lists, 7), ==, "7");
    g_assert_cmpstr(get_in_composed_list(lists, 8), ==, "8");
}

void remove_from_composed_list_test()
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
    g_assert_cmpstr(get_in_composed_list(lists, 0), ==, "1");
    g_assert_cmpstr(get_in_composed_list(lists, 1), ==, "2");
    g_ptr_array_insert(list1, 0, "0");

    delete_from_composed_list(lists, 1);
    g_assert_cmpstr(get_in_composed_list(lists, 0), ==, "0");
    g_assert_cmpstr(get_in_composed_list(lists, 1), ==, "2");
    g_ptr_array_insert(list1, 1, "1");

    delete_from_composed_list(lists, 2);
    g_assert_cmpstr(get_in_composed_list(lists, 1), ==, "1");
    g_assert_cmpstr(get_in_composed_list(lists, 2), ==, "3");
    g_ptr_array_insert(list2, 0, "2");

    delete_from_composed_list(lists, 3);
    g_assert_cmpstr(get_in_composed_list(lists, 2), ==, "2");
    g_assert_cmpstr(get_in_composed_list(lists, 3), ==, "4");
    g_ptr_array_insert(list2, 1, "3");

    delete_from_composed_list(lists, 7);
    g_assert_cmpstr(get_in_composed_list(lists, 6), ==, "6");
    g_assert_cmpstr(get_in_composed_list(lists, 7), ==, "8");
    g_ptr_array_insert(list3, 1, "7");

    delete_from_composed_list(lists, 8);
    g_assert_cmpstr(get_in_composed_list(lists, 7), ==, "7");
    g_assert_cmpstr(get_in_composed_list(lists, 8), ==, "9");
    g_ptr_array_insert(list3, 2, "8");
}

void wlr_list_remove_in_composed_list_test()
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
    g_assert_cmpstr(get_in_composed_list(lists, 1), ==, "2");
}

void wlr_list_find_in_composed_list_test()
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
    g_assert_cmpint(position, ==, 1);
}

void cross_sum_test()
{
    g_assert_cmpint(cross_sum(0b111, 2), ==, 3);
    g_assert_cmpint(cross_sum(100, 10), ==, 1);
}

void get_relative_item_in_list_test()
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

    g_assert_cmpstr(get_relative_item_in_list(list1, 0, -1), ==, "1");
    g_assert_cmpstr(get_relative_item_in_list(list2, 0, -1), ==, "5");
    g_assert_cmpstr(get_relative_item_in_list(list3, 0, -1), ==, "8");

    g_assert_cmpstr(get_relative_item_in_composed_list(lists, 0, 1), ==, "1");
    g_assert_cmpstr(get_relative_item_in_composed_list(lists, 3, 4), ==, "7");
    g_assert_cmpstr(get_relative_item_in_composed_list(lists, 3, -2), ==, "1");
}

void lower_bound_test()
{
    int base[6] = {1, 4, 5, 6, 8, 9};

    int key1 = 2;
    int low1 = lower_bound(&key1, base, 6, sizeof(int), cmp_int);
    g_assert_cmpint(low1, ==, 0);

    int key2 = 0;
    int low2 = lower_bound(&key2, base, 6, sizeof(int), cmp_int);
    g_assert_cmpint(low2, ==, -1);

    int key3 = 5;
    int low3 = lower_bound(&key3, base, 6, sizeof(int), cmp_int);
    g_assert_cmpint(low3, ==, 1);

    int key5 = 100;
    int low5 = lower_bound(&key5, base, 6, sizeof(int), cmp_int);
    g_assert_cmpint(low5, ==, 5);

    int key6 = -100;
    int low6 = lower_bound(&key6, base, 6, sizeof(int), cmp_int);
    g_assert_cmpint(low6, ==, -1);
}

void test_list_insert()
{
    GPtrArray *list = g_ptr_array_new();
    char *i = "0";
    char *j = "1";
    char *k = "2";
    g_ptr_array_insert(list, -1, i);
    g_ptr_array_insert(list, 100, j);
    g_ptr_array_insert(list, 1, k);

    g_assert_cmpstr(g_ptr_array_index(list, 0), ==, i);
    g_assert_cmpstr(g_ptr_array_index(list, 1), ==, j);
    g_assert_cmpstr(g_ptr_array_index(list, 2), ==, k);

    g_ptr_array_unref(list);
}

#define PREFIX "coreUtils"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(dir_exists_test);
    add_test(get_on_composed_list_test);
    add_test(remove_from_composed_list_test);
    add_test(wlr_list_remove_in_composed_list_test);
    add_test(wlr_list_find_in_composed_list_test);
    add_test(cross_sum_test);
    add_test(get_relative_item_in_list_test);
    add_test(lower_bound_test);

    return g_test_run();
}
