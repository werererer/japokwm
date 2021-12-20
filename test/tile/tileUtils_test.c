#include <stdlib.h>
#include <glib.h>

#include "server.h"
#include "tile/tileUtils.h"

void get_container_count_test()
{
    /* struct wlr_list tag_names; */
    /* wlr_list_init(&tag_names); */
    /* wlr_list_push(&tag_names, "1"); */
    /* wlr_list_push(&tag_names, "2"); */
    /* create_tags(&server.tags, &tag_names); */

    /* struct monitor m0, m1; */
    /* struct tagset *tagset0 = get_tagset_from_tag_id(&server.tags, 0); */
    /* tagset0->m = &m0; */
    /* struct tagset *tagset1 = get_tagset_from_tag_id(&server.tags, 1); */
    /* tagset1->m = &m1; */

    /* const int container_count = 3; */
    /* struct client clients[container_count]; */
    /* for (int i = 0; i < container_count; i++) { */
    /*     clients[0].type = XDG_SHELL; */
    /*     clients[0].sticky = false; */
    /* } */

    /* struct container cons[container_count]; */
    /* for (int i = 0; i < container_count; i++) { */
    /*     cons[i].client = &clients[i]; */
    /*     cons[i].floating = false; */
    /*     cons[i].m = &m0; */
    /* } */

    /* clients[0].ws_id = 0; */
    /* wlr_list_push(&tagset0->list_set.tiled_containers, &cons[0]); */
    /* clients[1].ws_id = 0; */
    /* wlr_list_push(&tagset0->list_set.tiled_containers, &cons[1]); */
    /* clients[2].ws_id = 1; */
    /* wlr_list_push(&tagset1->list_set.tiled_containers, &cons[2]); */
    /* ck_assert_int_eq(get_container_count(tagset0), 2); */
}

void get_relative_item_test()
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

    g_assert_cmpstr(get_relative_item_in_composed_list(lists, 4, 1), ==, "5");
}

#define PREFIX "tileUtils"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);


    add_test(get_container_count_test);
    add_test(get_relative_item_test);

    return g_test_run();
}
