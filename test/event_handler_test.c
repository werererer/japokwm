
#include <stdio.h>
#include <glib.h>

#include <lua.h>
#include "event_handler.h"

void test_emit_signal()
{
    struct event_handler *event_handler = create_event_handler();

    /* lua_State *L = luaL_newstate(); */

    destroy_event_handler(event_handler);
}

#define PREFIX "event_handler"
#define add_test(func) g_test_add_func("/"PREFIX"/"#func, func)
int main(int argc, char** argv)
{
    setbuf(stdout, NULL);
    g_test_init(&argc, &argv, NULL);

    add_test(test_emit_signal);

    return g_test_run();
}
