#include <glib.h>
#include <stdlib.h>

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

    return g_test_run();
}
