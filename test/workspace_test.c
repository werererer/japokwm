#include <check.h>
#include <stdio.h>

#include "workspace.h"

void workspace_contains_client_crash_test()
{
    /* workspace_contains_client(NULL, NULL); */
}

void workspace_has_clients_test()
{
    /* struct workspace *ws = create_workspace("test", 3); */
    /* ck_assert_int_eq(workspace_has_clients(ws), false); */
}

#define PREFIX "workspace"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(workspace_has_clients_test);
    add_test(workspace_contains_client_crash_test);

    return g_test_run();
}
