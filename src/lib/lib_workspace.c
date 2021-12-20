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
#include "lib/lib_container_list.h"
#include "lib/lib_list2D.h"
#include "lib/lib_layout.h"
#include "lib/lib_bitset.h"
#include "lib/lib_focus_set.h"
#include "lib/lib_direction.h"
#include "ipc-server.h"
#include "root.h"

static const struct luaL_Reg workspace_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg workspace_static_getter[] =
{
    {"focused", lib_workspace_get_focused},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_static_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg workspace_f[] =
{
    {"get", lib_workspace_get},
    {"get_next_empty", lib_workspace_get_next_empty},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_m[] =
{
    {"get_id", lib_workspace_get_id},
    {"swap", lib_workspace_swap},
    {"swap_smart", lib_workspace_swap_smart},
    {"toggle_bars", lib_workspace_toggle_bars},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_setter[] =
{
    {"tags", lib_set_tags},
    {"bars", lib_workspace_set_bars_visibility},
    {NULL, NULL},
};

static const struct luaL_Reg workspace_getter[] =
{
    {"bars", lib_workspace_get_bars_visibility},
    {"focus_set", lib_workspace_get_focus_set},
    {"focus_stack", lib_workspace_get_focus_stack},
    {"layout", lib_workspace_get_layout},
    {"prev_layout", lib_workspace_get_previous_layout},
    {"stack", lib_workspace_get_stack},
    {"tags", lib_workspace_get_tags},
    {"visible_focus_set", lib_workspace_get_visible_focus_set},
    {"visible_focus_stack", lib_workspace_get_visible_focus_stack},
    {"visible_stack", lib_workspace_get_visible_stack},
    {NULL, NULL},
};

void create_lua_workspace(lua_State *L, struct tag *ws)
{
    if (!ws)
        return;
    struct tag **user_con = lua_newuserdata(L, sizeof(struct tag*));
    *user_con = ws;

    luaL_setmetatable(L, CONFIG_WORKSPACE);
}

void lua_load_workspace(lua_State *L)
{
    create_class(L,
            workspace_meta,
            workspace_f,
            workspace_m,
            workspace_setter,
            workspace_getter,
            CONFIG_WORKSPACE);

    create_static_accessor(L,
            "Workspace",
            workspace_f,
            workspace_static_setter,
            workspace_static_getter);
}

struct tag *check_workspace(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_WORKSPACE);
    luaL_argcheck(L, ud != NULL, 1, "`container' expected");
    return (struct tag *)*ud;
}

// functions
int lib_workspace_get_next_empty(lua_State *L)
{
    enum wlr_direction dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    int ws_id = ws->id;
    switch (dir) {
        case WLR_DIRECTION_LEFT:
            ws = get_prev_empty_workspace(server_get_workspaces(), ws_id);
            break;
        case WLR_DIRECTION_RIGHT:
            ws = get_next_empty_workspace(server_get_workspaces(), ws_id);
            break;
        default:
            if (dir & WLR_DIRECTION_LEFT && dir & WLR_DIRECTION_RIGHT) {
                ws = get_nearest_empty_workspace(server_get_workspaces(), ws_id);
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

    struct tag *ws = get_workspace(ws_id);
    create_lua_workspace(L, ws);
    return 1;
}

// methods
int lib_workspace_swap(lua_State *L)
{
    struct tag *ws2 = check_workspace(L, 2);
    lua_pop(L, 1);

    struct tag *ws1 = check_workspace(L, 1);
    lua_pop(L, 1);

    workspace_swap(ws1, ws2);

    struct monitor *m = server_get_selected_monitor();
    struct tag *ws = monitor_get_active_workspace(m);
    tagset_reload(ws);
    arrange();
    workspace_update_names(server_get_workspaces());
    focus_most_recent_container();
    root_damage_whole(m->root);
    ipc_event_workspace();
    return 0;
}

int lib_workspace_swap_smart(lua_State *L)
{
    struct tag *ws2 = check_workspace(L, 2);
    lua_pop(L, 1);

    struct tag *ws1 = check_workspace(L, 1);
    lua_pop(L, 1);

    workspace_swap_smart(ws1, ws2);

    struct monitor *m = server_get_selected_monitor();
    struct tag *ws = monitor_get_active_workspace(m);
    tagset_reload(ws);
    arrange();
    workspace_update_names(server_get_workspaces());
    focus_most_recent_container();
    root_damage_whole(m->root);
    ipc_event_workspace();
    return 0;
}

int lib_workspace_get_id(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, ws->id);
    return 1;
}

int lib_workspace_toggle_bars(lua_State *L)
{
    enum wlr_edges visible_edges =
            WLR_EDGE_BOTTOM |
            WLR_EDGE_RIGHT |
            WLR_EDGE_LEFT |
            WLR_EDGE_TOP;
    if (lua_gettop(L) >= 2) {
        visible_edges = luaL_checkinteger(L, 2);
        lua_pop(L, 1);
    }
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);
    toggle_bars_visible(ws, visible_edges);
    return 0;
}

// setter
int lib_set_tags(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    tagset_set_tags(ws, bitset);
    return 0;
}

int lib_workspace_set_bars_visibility(lua_State *L)
{
    enum wlr_edges dir = luaL_checkinteger(L, 2);
    lua_pop(L, 1);
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    set_bars_visible(ws, dir);
    return 0;
}

// getter
int lib_workspace_get_focused(lua_State *L)
{
    struct tag *ws = server_get_selected_workspace();

    create_lua_workspace(L, ws);
    return 1;
}

int lib_workspace_get_bars_visibility(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, ws->visible_bar_edges);
    return 1;
}

int lib_workspace_get_focus_set(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_focus_set(L, ws->focus_set);
    return 1;
}

int lib_workspace_get_focus_stack(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, ws->focus_set->focus_stack_normal);
    return 1;
}

int lib_workspace_get_layout(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    struct layout *lt = workspace_get_layout(ws);
    create_lua_layout(L, lt);
    return 1;
}

int lib_workspace_get_previous_layout(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    struct layout *prev_layout = workspace_get_previous_layout(ws);
    create_lua_layout(L, prev_layout);
    return 1;
}

int lib_workspace_get_stack(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, ws->con_set->tiled_containers);
    return 1;
}

int lib_workspace_get_tags(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    BitSet *workspaces = ws->tags;
    create_lua_bitset_with_workspace(L, workspaces);
    return 1;
}

int lib_workspace_get_visible_focus_set(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_focus_set(L, ws->visible_focus_set);
    return 1;
}

int lib_workspace_get_visible_focus_stack(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, ws->visible_focus_set->focus_stack_normal);
    return 1;
}

int lib_workspace_get_visible_stack(lua_State *L)
{
    struct tag *ws = check_workspace(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, ws->visible_con_set->tiled_containers);
    return 1;
}
