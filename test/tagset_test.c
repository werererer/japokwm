#include <glib.h>
#include <stdio.h>

#include "monitor.h"
#include "tagset.h"
#include "workspace.h"

#define WORKSPACE_COUNT 4

void tagset_connect_workspace_test()
{
    struct tagset tagset1;
    /* struct workspace ws[WORKSPACE_COUNT]; */

/*     ws[0].tagset = NULL; */
/*     ws[0].selected_tagset = NULL; */

    tagset1.workspaces = bitset_create(WORKSPACE_COUNT);

    bitset_set(tagset1.workspaces, 0);

    bitset_destroy(tagset1.workspaces);
}

#define PREFIX "tagset"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(tagset_connect_workspace_test);

    return g_test_run();
}
