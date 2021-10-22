#include "lib/lib_workspace.h"

#include "client.h"
#include "container.h"
#include "list_sets/container_stack_set.h"
#include "monitor.h"
#include "root.h"
#include "server.h"
#include "tagset.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "workspace.h"
#include "list_sets/focus_stack_set.h"
#include "lib/lib_container.h"
#include "list_sets/list_set.h"
#include "lib/lib_list.h"
#include "lib/lib_list2D.h"
#include "lib/lib_layout.h"
#include "lib/lib_bitset.h"

static const struct luaL_Reg workspace_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg workspace_f[] =
{
    {"get", lib_workspace_get},
    {"get_focused", lib_workspace_get_focused},
    {"get_next_empty", lib_workspace_get_next_empty},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_m[] =
{
    {"get_id", lib_workspace_get_id},
    {"swap", lib_workspace_swap},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_setter[] =
{
    {"tags", lib_set_tags},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_getter[] =
{
    {"focus_stack", lib_workspace_get_focus_stack},
    {"layout", lib_workspace_get_layout},
    {"stack", lib_workspace_get_stack},
    {"tags", lib_workspace_get_tags},
    {NULL, NULL},
};

void create_lua_workspace(lua_State *L, struct workspace *ws)
{
    if (!ws)
        return;
    struct workspace **user_con = lua_newuserdata(L, sizeof(struct workspace*));
    *user_con = ws;

    luaL_setmetatable(L, CONFIG_WORKSPACE);
}

void lua_load_workspace()
{
    create_class(
            workspace_meta,
            workspace_f,
            workspace_m,
            workspace_setter,
            workspace_getter,
            CONFIG_WORKSPACE);

    luaL_newlib(L, workspace_f);
    lua_setglobal(L, "Workspace");
}

struct workspace *check_workspace(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_WORKSPACE);
    luaL_argcheck(L, ud != NULL, 1, "`container' expected");
    return (struct workspace *)*ud;
}

// functions
int lib_workspace_get_next_empty(lua_State *L)
{
    enum wlr_direction dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    int ws_id = ws->id;
    switch (dir) {
        case WLR_DIRECTION_LEFT:
            ws = get_prev_empty_workspace(server.workspaces, ws_id);
            break;
        case WLR_DIRECTION_RIGHT:
            ws = get_next_empty_workspace(server.workspaces, ws_id);
            break;
        default:
            if (dir & WLR_DIRECTION_LEFT && dir & WLR_DIRECTION_RIGHT) {
                ws = get_nearest_empty_workspace(server.workspaces, ws_id);
            } else {
                ws = get_workspace(ws_id);
            }
    }

    create_lua_workspace(L, ws);
    return 1;
}

int lib_workspace_get(lua_State *L)
{
    int ws_id = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);

    struct workspace *ws = get_workspace(ws_id);
    create_lua_workspace(L, ws);
    return 1;
}

int lib_workspace_get_focused(lua_State *L)
{
    struct workspace *ws = server_get_selected_workspace();

    create_lua_workspace(L, ws);
    return 1;
}

// methods
int lib_workspace_swap(lua_State *L)
{
    struct workspace *ws2 = check_workspace(L, 2);
    lua_pop(L, 1);

    struct workspace *ws1 = check_workspace(L, 1);
    lua_pop(L, 1);

    GPtrArray *future_ws2_containers = g_ptr_array_new();
    for (int i = 0; i < ws1->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(ws1->con_set->tiled_containers, i);
        struct monitor *ws_m = workspace_get_monitor(ws1);
        if (!exist_on(ws_m, ws1->workspaces, con))
            continue;

        g_ptr_array_add(future_ws2_containers, con);
    }

    for (int i = 0; i < ws2->con_set->tiled_containers->len; i++) {
        struct container *con = g_ptr_array_index(ws2->con_set->tiled_containers, i);
        struct monitor *ws_m = workspace_get_monitor(ws2);
        if (!exist_on(ws_m, ws2->workspaces, con))
            continue;
        con->ws_id = ws1->id;
        bitset_reset_all(con->client->sticky_workspaces);
        bitset_set(con->client->sticky_workspaces, con->ws_id);
    }

    for (int i = 0; i < future_ws2_containers->len; i++) {
        struct container *con = g_ptr_array_index(future_ws2_containers, i);
        con->ws_id = ws2->id;
        bitset_reset_all(con->client->sticky_workspaces);
        bitset_set(con->client->sticky_workspaces, con->ws_id);
    }
    g_ptr_array_unref(future_ws2_containers);

    struct monitor *m = server_get_selected_monitor();
    struct workspace *ws = monitor_get_active_workspace(m);
    tagset_reload(ws);
    arrange();
    focus_most_recent_container();
    root_damage_whole(m->root);
    return 0;
}

int lib_workspace_get_id(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, ws->id);
    return 1;
}

// setter

int lib_set_tags(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    tagset_set_tags(ws, bitset);
    return 0;
}
// getter
int lib_workspace_get_focus_stack(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_list2D(L, ws->focus_set->focus_stack_lists);
    return 1;
}

int lib_workspace_get_layout(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    struct layout *lt = workspace_get_layout(ws);
    create_lua_layout(L, lt);
    return 1;
}

int lib_workspace_get_stack(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_list(L, ws->con_set->tiled_containers);
    return 1;
}

int lib_workspace_get_tags(lua_State *L)
{
    struct workspace *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    BitSet *workspaces = ws->workspaces;
    create_lua_bitset_with_workspace(workspaces);
    return 1;
}
