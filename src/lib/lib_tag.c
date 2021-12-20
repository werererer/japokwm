#include "lib/lib_tag.h"

#include "client.h"
#include "container.h"
#include "list_sets/container_stack_set.h"
#include "monitor.h"
#include "root.h"
#include "server.h"
#include "tagset.h"
#include "tile/tileUtils.h"
#include "translationLayer.h"
#include "tag.h"
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

static const struct luaL_Reg tag_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg tag_static_getter[] =
{
    {"focused", lib_tag_get_focused},
    {NULL, NULL},
};

static const struct luaL_Reg tag_static_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg tag_f[] =
{
    {"get", lib_tag_get},
    {"get_next_empty", lib_tag_get_next_empty},
    {NULL, NULL},
};

static const struct luaL_Reg tag_m[] =
{
    {"get_id", lib_tag_get_id},
    {"swap", lib_tag_swap},
    {"swap_smart", lib_tag_swap_smart},
    {"toggle_bars", lib_tag_toggle_bars},
    {NULL, NULL},
};

static const struct luaL_Reg tag_setter[] =
{
    {"tags", lib_set_tags},
    {"bars", lib_tag_set_bars_visibility},
    {NULL, NULL},
};

static const struct luaL_Reg tag_getter[] =
{
    {"bars", lib_tag_get_bars_visibility},
    {"focus_set", lib_tag_get_focus_set},
    {"focus_stack", lib_tag_get_focus_stack},
    {"layout", lib_tag_get_layout},
    {"prev_layout", lib_tag_get_previous_layout},
    {"stack", lib_tag_get_stack},
    {"tags", lib_tag_get_tags},
    {"visible_focus_set", lib_tag_get_visible_focus_set},
    {"visible_focus_stack", lib_tag_get_visible_focus_stack},
    {"visible_stack", lib_tag_get_visible_stack},
    {NULL, NULL},
};

void create_lua_tag(lua_State *L, struct tag *tag)
{
    if (!tag)
        return;
    struct tag **user_con = lua_newuserdata(L, sizeof(struct tag*));
    *user_con = tag;

    luaL_setmetatable(L, CONFIG_tag);
}

void lua_load_tag(lua_State *L)
{
    create_class(L,
            tag_meta,
            tag_f,
            tag_m,
            tag_setter,
            tag_getter,
            CONFIG_tag);

    create_static_accessor(L,
            "Tag",
            tag_f,
            tag_static_setter,
            tag_static_getter);
}

struct tag *check_tag(lua_State *L, int narg) {
    void **ud = luaL_checkudata(L, narg, CONFIG_tag);
    luaL_argcheck(L, ud != NULL, 1, "`container' expected");
    return (struct tag *)*ud;
}

// functions
int lib_tag_get_next_empty(lua_State *L)
{
    enum wlr_direction dir = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    int ws_id = tag->id;
    switch (dir) {
        case WLR_DIRECTION_LEFT:
            tag = get_prev_empty_tag(server_get_tags(), ws_id);
            break;
        case WLR_DIRECTION_RIGHT:
            tag = get_next_empty_tag(server_get_tags(), ws_id);
            break;
        default:
            if (dir & WLR_DIRECTION_LEFT && dir & WLR_DIRECTION_RIGHT) {
                tag = get_nearest_empty_tag(server_get_tags(), ws_id);
            } else {
                tag = get_tag(ws_id);
            }
    }

    create_lua_tag(L, tag);
    return 1;
}

int lib_tag_get(lua_State *L)
{
    int ws_id = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);

    struct tag *tag = get_tag(ws_id);
    create_lua_tag(L, tag);
    return 1;
}

// methods
int lib_tag_swap(lua_State *L)
{
    struct tag *tag2 = check_tag(L, 2);
    lua_pop(L, 1);

    struct tag *tag1 = check_tag(L, 1);
    lua_pop(L, 1);

    tag_swap(tag1, tag2);

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    tagset_reload(tag);
    arrange();
    tag_update_names(server_get_tags());
    focus_most_recent_container();
    root_damage_whole(m->root);
    ipc_event_tag();
    return 0;
}

int lib_tag_swap_smart(lua_State *L)
{
    struct tag *tag2 = check_tag(L, 2);
    lua_pop(L, 1);

    struct tag *tag1 = check_tag(L, 1);
    lua_pop(L, 1);

    tag_swap_smart(tag1, tag2);

    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    tagset_reload(tag);
    arrange();
    tag_update_names(server_get_tags());
    focus_most_recent_container();
    root_damage_whole(m->root);
    ipc_event_tag();
    return 0;
}

int lib_tag_get_id(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, tag->id);
    return 1;
}

int lib_tag_toggle_bars(lua_State *L)
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
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);
    toggle_bars_visible(tag, visible_edges);
    return 0;
}

// setter
int lib_set_tags(lua_State *L)
{
    BitSet *bitset = check_bitset(L, 2);
    lua_pop(L, 1);

    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    tagset_set_tags(tag, bitset);
    return 0;
}

int lib_tag_set_bars_visibility(lua_State *L)
{
    enum wlr_edges dir = luaL_checkinteger(L, 2);
    lua_pop(L, 1);
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    set_bars_visible(tag, dir);
    return 0;
}

// getter
int lib_tag_get_focused(lua_State *L)
{
    struct tag *tag = server_get_selected_tag();

    create_lua_tag(L, tag);
    return 1;
}

int lib_tag_get_bars_visibility(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    lua_pushinteger(L, tag->visible_bar_edges);
    return 1;
}

int lib_tag_get_focus_set(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    create_lua_focus_set(L, tag->focus_set);
    return 1;
}

int lib_tag_get_focus_stack(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, tag->focus_set->focus_stack_normal);
    return 1;
}

int lib_tag_get_layout(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    struct layout *lt = tag_get_layout(tag);
    create_lua_layout(L, lt);
    return 1;
}

int lib_tag_get_previous_layout(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    struct layout *prev_layout = tag_get_previous_layout(tag);
    create_lua_layout(L, prev_layout);
    return 1;
}

int lib_tag_get_stack(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, tag->con_set->tiled_containers);
    return 1;
}

int lib_tag_get_tags(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    BitSet *tags = tag->tags;
    create_lua_bitset_with_tag(L, tags);
    return 1;
}

int lib_tag_get_visible_focus_set(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    create_lua_focus_set(L, tag->visible_focus_set);
    return 1;
}

int lib_tag_get_visible_focus_stack(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, tag->visible_focus_set->focus_stack_normal);
    return 1;
}

int lib_tag_get_visible_stack(lua_State *L)
{
    struct tag *tag = check_tag(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, tag->visible_con_set->tiled_containers);
    return 1;
}
