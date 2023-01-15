#include <stdio.h>

#include "server.h"
#include "tag.h"

void test_tag_update_tag_name()
{
    init_server();

    struct tag *tag = get_tag(0);
    tag->name = strdup("gibberish");
    tag_update_name(tag);
    g_assert_cmpstr(tag->name, ==, "0:1");
    free(tag->name);
}

#define PREFIX "tag"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(test_tag_update_tag_name);

    return g_test_run();
}
