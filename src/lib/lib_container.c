#include "lib/lib_container.h"

#include "bitset/bitset.h"
#include "client.h"
#include "container.h"
#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "workspace.h"
#include "lib/lib_workspace.h"
#include "lib/lib_geom.h"
#include "lib/lib_container_property.h"
#include "bitset/bitset.h"
#include "lib/lib_bitset.h"
#include "lib/lib_container_property_list.h"

static const struct luaL_Reg container_meta[] =
{
    {"__eq", lib_container_is_equal},
    {NULL, NULL},
};

static const struct luaL_Reg container_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg container_static_getter[] =
{
    {"focused", lib_container_get_focused},
    {NULL, NULL},
};

static const struct luaL_Reg container_static_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg container_m[] = {
    {"focus", lib_container_focus},
    {"kill", lib_container_kill},
    {"toggle_add_sticky", lib_container_toggle_add_sticky},
    {"toggle_add_sticky_restricted", lib_container_toggle_add_sticky_restricted},
    {NULL, NULL},
};

static const struct luaL_Reg container_getter[] = {
    // TODO: implement getters
    {"alpha", lib_container_get_alpha},
    {"app_id", lib_container_get_app_id},
    {"properties", lib_container_get_property},
    {"property", lib_container_get_current_property},
    {"sticky", lib_container_set_sticky},
    {"workspace", lib_container_get_workspace},
    {NULL, NULL},
};

static const struct luaL_Reg container_setter[] = {
    {"alpha", lib_container_set_alpha},
    {"ratio", lib_container_set_ratio},
    {"sticky", lib_container_set_sticky},
    {"sticky_restricted", lib_container_set_sticky_restricted},
    {"workspace", lib_container_move_to_workspace},
    {NULL, NULL},
};

void create_lua_container(lua_State *L, struct container *con)
{
    if (!con) {
        lua_pushnil(L);
        return;
    }
    struct container **user_con = lua_newuserdata(L, sizeof(struct container*));
    *user_con = con;

    luaL_setmetatable(L, CONFIG_CONTAINER);
}

void lua_load_container(lua_State *L)
{
    create_class(L,
            container_meta,
            container_f,
            container_m,
            container_setter,
            container_getter,
            CONFIG_CONTAINER);

    create_static_accessor(L,
            "Container",
            container_f,
            container_static_setter,
            container_static_getter);
}

struct container *check_container(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_CONTAINER);
    luaL_argcheck(L, ud != NULL, argn, "`container' expected");
    return (struct container *)*ud;
}

int lib_container_set_ratio(lua_State *L) {
    float ratio = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    con->ratio = ratio;
    return 0;
}

int lib_container_is_equal(lua_State *L) {
    struct container *con1 = check_container(L, 2);
    lua_pop(L, 1);
    struct container *con2 = check_container(L, 1);
    lua_pop(L, 1);
    bool con_is_equal = con1 == con2;
    lua_pushboolean(L, con_is_equal);
    return 1;
}

int lib_container_focus(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    focus_container(con);
    return 0;
}

int lib_container_move_to_workspace(lua_State *L)
{
    struct tag *ws = check_workspace(L, 2);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    move_container_to_workspace(con, ws);
    return 0;
}

int lib_container_set_alpha(lua_State *L)
{
    float alpha = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    if (!con)
        return 0;

    con->alpha = alpha;
    return 0;
}

int lib_container_set_sticky(lua_State *L) {
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_set_sticky_restricted(lua_State *L) {
    // TODO fix this function
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    bool is_sticky_at_ws_id = bitset_test(con->client->sticky_workspaces, con->ws_id);
    bitset_assign(bitset, con->ws_id, is_sticky_at_ws_id);

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_toggle_add_sticky(lua_State *L) {
    /* bool sticky = lua_toboolean(L, -1); */
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    bitset_xor(bitset, con->client->sticky_workspaces);

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_toggle_add_sticky_restricted(lua_State *L) {
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct tag *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);
    if (!con)
        return 0;

    bitset_xor(bitset, con->client->sticky_workspaces);
    bitset_set(bitset, bitset_test(con->client->sticky_workspaces, con->ws_id));

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_kill(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    if (!con)
        return 0;

    struct client *c = con->client;
    kill_client(c);
    return 0;
}

// getter
int lib_container_get_focused(lua_State *L) {
    struct monitor *m = server_get_selected_monitor();
    struct container *con = monitor_get_focused_container(m);

    create_lua_container(L, con);
    return 1;
}

int lib_container_get_alpha(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    lua_pushnumber(L, con->alpha);
    return 1;
}

int lib_container_get_app_id(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    const char *app_id = container_get_app_id(con);
    lua_pushstring(L, app_id);
    return 1;
}

int lib_container_get_current_property(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    struct container_property *con_property = container_get_property(con);

    create_lua_container_property(L, con_property);
    return 1;
}

int lib_container_get_property(lua_State *L)
{
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    create_lua_container_property_list(L, con->properties);
    return 1;
}

int lib_container_get_sticky(lua_State *L)
{
    // TODO implement me
    /* struct container *con = check_container(L); */
    /* lua_pop(L, 1); */

    /* lua_pushinteger(L, con->client->sticky_workspaces); */
    return 1;
}

int lib_container_get_workspace(lua_State *L) {
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    if (!con)
        return 0;

    struct tag *ws = container_get_workspace(con);
    create_lua_workspace(L, ws);
    return 1;
}
