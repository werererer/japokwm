#include <glib.h>
#include <stdio.h>

#define PREFIX "client"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    return g_test_run();
}
