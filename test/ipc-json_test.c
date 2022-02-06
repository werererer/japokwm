#include <glib.h>
#include <stdio.h>

#include "server.h"
#include "ipc-json.h"

void test_ipc_json_describe_tagsets()
{
    init_server();

    // struct tag *tag = get_tag(0);
    // ipc_json_describe_tagsets();
}

#define PREFIX "container"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);


    return g_test_run();
}
