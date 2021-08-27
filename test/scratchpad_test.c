#include <stdlib.h>
#include <glib.h>

#include "scratchpad.h"

void tagset_connect_workspace_testestace()
{
    /* show_scratchpad(); */
}

#define PREFIX "scratchpad"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(tagset_connect_workspace_testestace);

    return g_test_run();
}
