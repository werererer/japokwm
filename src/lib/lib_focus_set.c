#include "lib/lib_focus_set.h"

#include "server.h"
#include "list_sets/focus_stack_set.h"
#include "translationLayer.h"
#include "lib/lib_focus_set.h"
#include "lib/lib_list2D.h"
#include "lib/lib_container_list.h"

static const struct luaL_Reg focus_stack_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg focus_stack_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg focus_stack_m[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg focus_stack_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg focus_stack_getter[] =
{

    {"focus_stack_layer_background", lib_get_focus_stack_layer_background},
    {"focus_stack_layer_bottom", lib_get_focus_stack_layer_bottom},
    {"focus_stack_layer_overlay", lib_get_focus_stack_layer_overlay},
    {"focus_stack_layer_top", lib_get_focus_stack_layer_top},
    {"focus_stack_lists", lib_get_focus_stack_lists},
    {"focus_stack_lists_with_layer_shell", lib_get_focus_stack_lists_with_layer_shell},
    {"focus_stack_normal", lib_get_focus_stack_normal},
    {"focus_stack_not_focusable", lib_get_focus_stack_not_focusable},
    {"focus_stack_on_top", lib_get_focus_stack_on_top},
    {"focus_stack_visible_lists", lib_get_focus_stack_visible_lists},
    {NULL, NULL},
};

void lua_load_focus_set(lua_State *L)
{
    create_class(L,
            focus_stack_meta,
            focus_stack_f,
            focus_stack_m,
            focus_stack_setter,
            focus_stack_getter,
            CONFIG_FOCUS_SET);

    luaL_newlib(L, focus_stack_f);
    lua_setglobal(L, "Focus_set");
}

struct focus_set *check_focus_set(lua_State *L, int narg)
{
    void **ud = luaL_checkudata(L, narg, CONFIG_FOCUS_SET);
    luaL_argcheck(L, ud != NULL, narg, "`focus_set' expected");
    return *(struct focus_set **)ud;
}

void create_lua_focus_set(lua_State *L, struct focus_set *focus_set)
{
    struct focus_set **user_focus_set = lua_newuserdata(L, sizeof(struct focus_set));
    *user_focus_set = focus_set;

    luaL_setmetatable(L, CONFIG_FOCUS_SET);
}

int lib_get_focus_stack_lists_with_layer_shell(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_list2D(L, focus_set->focus_stack_lists_with_layer_shell);
    return 1;
}

int lib_get_focus_stack_visible_lists(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_list2D(L, focus_set->focus_stack_visible_lists);
    return 1;
}

int lib_get_focus_stack_lists(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_list2D(L, focus_set->focus_stack_lists);
    return 1;
}

int lib_get_focus_stack_layer_background(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_layer_background);
    return 1;
}

int lib_get_focus_stack_layer_bottom(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_layer_bottom);
    return 1;
}

int lib_get_focus_stack_layer_top(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_layer_top);
    return 1;
}

int lib_get_focus_stack_layer_overlay(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_layer_overlay);
    return 1;
}

int lib_get_focus_stack_on_top(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_on_top);
    return 1;
}

int lib_get_focus_stack_normal(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_normal);
    return 1;
}

int lib_get_focus_stack_not_focusable(lua_State *L)
{
    struct focus_set *focus_set = check_focus_set(L, 1);
    lua_pop(L, 1);

    create_lua_container_list(L, focus_set->focus_stack_not_focusable);
    return 1;
}
