#include "lib/actions/lib_container.h"
#include "container.h"
#include "client.h"
#include "monitor.h"
#include "tile/tileUtils.h"
#include "server.h"
#include "bitset/bitset.h"
#include "workspace.h"

int lib_container_set_ratio(lua_State *L)
{
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

int lib_container_get_workspace(lua_State *L)
{
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, position);

    if (!con)
        return 0;

    struct workspace *container_workspace = container_get_workspace(con);
    int ws_id = container_workspace->id;
    lua_pushinteger(L, ws_id);
    return 1;
}

int lib_container_set_alpha(lua_State *L)
{
    float alpha = luaL_checknumber(L, -1);
    lua_pop(L, 1);
    int position = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, position);

    if (!con)
        return 0;

    con->alpha = alpha;
    return 0;
}

int lib_container_set_sticky(lua_State *L)
{
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create(server.workspaces->len);
    for (int i = 0; i < bitset->size; i++) {
        int last_bit_id = tmp_bitset->size - 1;
        bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
    }
    bitset_destroy(tmp_bitset);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);
    debug_print("container set sticky: %p\n", con);
    for (int i = 0; i < bitset->size; i++) {
        debug_print("bit: %i\n", bitset_test(bitset, i));
    }

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_set_sticky_restricted(lua_State *L)
{
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create(server.workspaces->len);
    for (int i = 0; i < bitset->size; i++) {
        int last_bit_id = tmp_bitset->size - 1;
        bitset_assign(bitset, i, bitset_test(tmp_bitset, last_bit_id - i));
    }
    bitset_destroy(tmp_bitset);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    struct container *con = get_container(ws, i);

    bitset_set(bitset, bitset_test(con->client->sticky_workspaces, con->client->ws_id));

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_toggle_add_sticky(lua_State *L)
{
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create(server.workspaces->len);
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

    debug_print("container set sticky: %p\n", con);
    for (int i = 0; i < bitset->size; i++) {
        debug_print("bit: %i\n", bitset_test(bitset, i));
    }

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}

int lib_container_toggle_add_sticky_restricted(lua_State *L)
{
    // TODO fix this function
    /* bool sticky = lua_toboolean(L, -1); */
    uint64_t tags_dec = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    int i = luaL_checkinteger(L, -1);
    lua_pop(L, 1);

    BitSet *tmp_bitset = bitset_from_value(tags_dec);
    BitSet *bitset = bitset_create(server.workspaces->len);
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
    bitset_set(bitset, bitset_test(con->client->sticky_workspaces, con->client->ws_id));

    debug_print("container set sticky: %p\n", con);
    for (int i = 0; i < bitset->size; i++) {
        debug_print("bit: %i\n", bitset_test(bitset, i));
    }

    if (!con)
        return 0;

    client_setsticky(con->client, bitset);
    return 0;
}
