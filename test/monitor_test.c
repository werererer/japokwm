#include <stdio.h>
#include <glib.h>

#include "tag.h"
#include "monitor.h"
#include "server.h"

void assert_link_ok(struct monitor *m, struct tag *tag)
{
    if (!tag)
        return;
    g_assert_cmpint(m->tag_id, ==, tag->id);
    g_assert_cmpmem(tag->m, 1, m, 1);
}

void test_monitor_set_selected_tag()
{
    init_server();

    struct monitor m0 = {.tag_id = INVALID_TAG_ID};
    struct monitor m1 = {.tag_id = INVALID_TAG_ID};

    struct tag *tag0 = get_tag(0);
    struct tag *tag1 = get_tag(1);

    monitor_set_selected_tag(&m0, tag0);

    assert_link_ok(&m0, tag0);

    monitor_set_selected_tag(&m1, tag1);
    assert_link_ok(&m1, tag1);

    // we swap the tags now
    monitor_set_selected_tag(&m1, tag0);
    assert_link_ok(&m1, tag0);
    assert_link_ok(&m0, tag1);
}

void test2_monitor_set_selected_tag()
{
    init_server();

    struct monitor m0 = {.tag_id = INVALID_TAG_ID};
    struct monitor m1 = {.tag_id = INVALID_TAG_ID};

    struct tag *tag0 = get_tag(0);
    struct tag *tag1 = get_tag(1);

    monitor_set_selected_tag(&m0, tag0);

    assert_link_ok(&m0, tag0);
    assert_link_ok(&m1, NULL);

    monitor_set_selected_tag(&m1, tag0);
    assert_link_ok(&m0, tag0);
    assert_link_ok(&m1, tag1);
}

#define PREFIX "monitor"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char** argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(test_monitor_set_selected_tag);
    add_test(test2_monitor_set_selected_tag);

    return g_test_run();
}
