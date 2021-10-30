#include "lib/lib_server.h"

#include "server.h"
#include "translationLayer.h"
#include "lib/lib_workspace.h"
#include "lib/lib_ring_buffer.h"
#include "ring_buffer.h"

static const struct luaL_Reg server_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg server_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg server_m[] =
{
    {"get_workspace", lib_server_get_workspaces},
    {"get_focused_workspace", lib_server_get_focused_workspace},
    {"quit", lib_server_quit},
    {NULL, NULL},
};

static const struct luaL_Reg server_setter[] =
{
    {"default_layout_ring", lib_server_set_default_layout_ring},
    {NULL, NULL},
};

static const struct luaL_Reg server_getter[] =
{
    {"default_layout_ring", lib_server_get_default_layout_ring},
    {NULL, NULL},
};

void create_lua_server(struct server *server)
{
    if (!server)
        return;
    struct server **user_con = lua_newuserdata(L, sizeof(struct server*));
    *user_con = server;

    luaL_setmetatable(L, CONFIG_SERVER);
}

void lua_load_server(lua_State *L)
{
    create_class(L,
            server_meta,
            server_f,
            server_m,
            server_setter,
            server_getter,
            CONFIG_SERVER);

    create_lua_server(&server);
    lua_setglobal(L, "server");

    luaL_newlib(L, server_f);
    lua_setglobal(L, "Server");
}

static struct server *check_server(lua_State *L) {
    void **ud = luaL_checkudata(L, 1, CONFIG_SERVER);
    luaL_argcheck(L, ud != NULL, 1, "`container' expected");
    return (struct server *)*ud;
}

// functions

int lib_server_get_focused_workspace(lua_State *L)
{
    struct workspace *ws = server_get_selected_workspace();
    create_lua_workspace(L, ws);
    return 1;
}

int lib_server_get_workspaces(lua_State *L)
{
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    // TODO fixme use server variable instead
    check_server(L);
    lua_pop(L, 1);

    struct workspace *ws = get_workspace(i);
    create_lua_workspace(L, ws);
    return 1;
}

int lib_server_quit(lua_State *L)
{
    struct server *server = check_server(L);
    lua_pop(L, 1);
    wl_display_terminate(server->wl_display);
    return 0;
}

// getter
int lib_server_get_default_layout_ring(lua_State *L)
{
    create_lua_ring_buffer(L, server.default_layout_ring);
    return 1;
}
// setter
int lib_server_set_default_layout_ring(lua_State *L)
{
    struct ring_buffer *default_layout_ring = check_ring_buffer(L, 2);
    lua_pop(L, 1);
    struct server *server = check_server(L);
    lua_pop(L, 1);

    ring_buffer_set(server->default_layout_ring, default_layout_ring);
    lua_pop(L, 1);
    return 0;
}
