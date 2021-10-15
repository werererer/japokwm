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

static const struct luaL_Reg container_f[] =
{
    {"get_focused", lib_container_get_focused},
    {NULL, NULL},
};

static const struct luaL_Reg container_m[] = {
    {"toggle_add_sticky", lib_container_toggle_add_sticky},
    {"toggle_add_sticky_restricted",
     lib_container_toggle_add_sticky_restricted},
    {NULL, NULL},
};

static const struct luaL_Reg container_getter[] = {
    // TODO: implement getters
    {"alpha", lib_container_get_alpha},
    {"app_id", lib_container_get_app_id},
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

void lua_load_container()
{
    create_class(
            container_f,
            container_m,
            container_setter,
            container_getter,
            CONFIG_CONTAINER);

    luaL_newlib(L, container_f);
    lua_setglobal(L, "Container");
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
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, position);

    if (!con)
        return 0;

    con->ratio = ratio;
    return 0;
}

int lib_container_get_focused(lua_State *L) {
    struct monitor *m = server_get_selected_monitor();
    struct container *con = monitor_get_focused_container(m);

    create_lua_container(L, con);
    return 1;
}

int lib_container_move_to_workspace(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 2);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    move_container_to_workspace(con, ws);
    return 0;
}

int lib_container_set_alpha(lua_State *L) {
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
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create();
    for (int i = 0; i < bitset->size; i++) {
        int last_bit_id = tmp_bitset->size - 1;
        bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
    }
    bitset_destroy(tmp_bitset);

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_set_sticky_restricted(lua_State *L) {
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create();
    for (int i = 0; i < bitset->size; i++) {
        int last_bit_id = tmp_bitset->size - 1;
        bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
    }
    bitset_destroy(tmp_bitset);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);

    bitset_set(bitset,
            bitset_test(con->client->sticky_workspaces, con->client->ws_id));

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_toggle_add_sticky(lua_State *L) {
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct container *con = check_container(L, 1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create();
    for (int i = 0; i < bitset->size; i++) {
        int last_bit_id = tmp_bitset->size - 1;
        bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
    }
    bitset_destroy(tmp_bitset);

    if (!con)
        return 0;

    bitset_xor(bitset, con->client->sticky_workspaces);

    debug_print("container set sticky: %p\n", con);
    for (int i = 0; i < bitset->size; i++) {
        debug_print("bit: %i\n", bitset_test(bitset, i));
    }

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_toggle_add_sticky_restricted(lua_State *L) {
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create();
    for (int i = 0; i < bitset->size; i++) {
        int last_bit_id = tmp_bitset->size - 1;
        bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
    }
    bitset_destroy(tmp_bitset);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);
    if (!con)
        return 0;

    bitset_xor(bitset, con->client->sticky_workspaces);
    bitset_set(bitset,
            bitset_test(con->client->sticky_workspaces, con->client->ws_id));

    debug_print("container set sticky: %p\n", con);
    for (int i = 0; i < bitset->size; i++) {
        debug_print("bit: %i\n", bitset_test(bitset, i));
    }

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

// getter
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

    struct workspace *ws = container_get_workspace(con);
    create_lua_workspace(L, ws);
    return 1;
}
