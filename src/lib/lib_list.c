#include "lib/lib_list.h"

#include "translationLayer.h"
#include "server.h"
#include "utils/coreUtils.h"
#include "tile/tileUtils.h"
#include "tagset.h"
#include "tag.h"

#include <GLES2/gl2.h>
#include <stdlib.h>
#include <ctype.h>

struct user_list {
    GPtrArray *list;
    void (*create_lua_object)(lua_State *L, void *item);
    void *(*check_object)(lua_State *L, int narg);
};

static const struct luaL_Reg list_meta[] =
{
    {"__index", lib_list_get},
    {"__gc", lib_list_gc},
    {NULL, NULL},
};

static const struct luaL_Reg list_f[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg list_m[] = {
    {"find", lib_list_find},
    {"repush", lib_list_repush},
    {"swap", lib_list_swap},
    {NULL, NULL},
};

static const struct luaL_Reg list_getter[] = {
    {"len", lib_list_length},
    {NULL, NULL},
};

static const struct luaL_Reg list_setter[] = {
    {NULL, NULL},
};

static struct user_list *create_user_list(
        GPtrArray *list,
        void *check_object(lua_State *L, int nagr),
        void create_lua_object(lua_State *L, void *obj))
{
    struct user_list *user_list = calloc(1, sizeof(struct user_list));
    user_list->list = list;
    user_list->check_object = check_object;
    user_list->create_lua_object = create_lua_object;
    return user_list;
}

static void destroy_user_list(struct user_list *user_list)
{
    free(user_list);
}

void create_lua_list(
        lua_State *L,
        GPtrArray *arr,
        void create_lua_object(lua_State *L, void *),
        void *check_object(lua_State *L, int narg))
{
    if (!arr) {
        lua_pushnil(L);
        return;
    }

    struct user_list *list = create_user_list(arr, check_object, create_lua_object);
    struct user_list **user_list = lua_newuserdata(L, sizeof(struct user_list *));
    *user_list = list;

    luaL_setmetatable(L, CONFIG_LIST);
}

void lua_load_list(lua_State *L)
{
    create_class(L,
            list_meta,
            list_f,
            list_m,
            list_setter,
            list_getter,
            CONFIG_LIST);

    luaL_newlib(L, list_f);
    lua_setglobal(L, "List");
}

struct user_list *check_user_list(lua_State *L, int argn)
{
    void **ud = luaL_checkudata(L, argn, CONFIG_LIST);
    luaL_argcheck(L, ud != NULL, argn, "`list' expected");
    return (struct user_list *)*ud;
}

GPtrArray *check_list(lua_State *L, int argn)
{
    struct user_list *user_list = check_user_list(L, argn);
    return user_list->list;
}

int lib_list_gc(lua_State *L)
{
    struct user_list *user_list = check_user_list(L, 1);
    lua_pop(L, 1);
    destroy_user_list(user_list);
    return 0;
}

// static methods
// methods
int lib_list_find(lua_State *L)
{
    struct user_list *user_list = check_user_list(L, 1);
    void *item = user_list->check_object(L, 2);
    lua_pop(L, 2);

    guint pos;
    g_ptr_array_find(user_list->list, item, &pos);

    lua_pushinteger(L, c_idx_to_lua_idx(pos));
    return 1;
}

int lib_list_get(lua_State *L)
{
    struct user_list *user_list = check_user_list(L, 1);

    bool is_number = lua_isnumber(L, -1);
    if (!is_number) {
        get_lua_value(L);
        return 1;
    }

    int i = lua_idx_to_c_idx(lua_tonumber(L, -1));
    if (i < 0) {
        lua_pushnil(L);
        return 1;
    }
    if (i >= user_list->list->len) {
        lua_pushnil(L);
        return 1;
    }

    void *con = g_ptr_array_index(user_list->list, i);
    user_list->create_lua_object(L, con);
    return 1;
}

int lib_list_swap(lua_State *L)
{
    int j = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
    int i = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
    GPtrArray *array = check_list(L, 1);
    lua_pop(L, 1);

    tag_swap_containers(array, i, j);

    struct tag *tag = server_get_selected_tag();
    tagset_reload(tag);
    arrange();
    return 0;
}

int lib_list_repush(lua_State *L)
{
    int abs_index = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
    int i = lua_idx_to_c_idx(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
    GPtrArray *array = check_list(L, 1);
    lua_pop(L, 1);

    tag_repush_containers(array, i, abs_index);

    struct tag *tag = server_get_selected_tag();
    tagset_reload(tag);
    arrange();
    return 0;
}

// getter
int lib_list_length(lua_State *L)
{
    GPtrArray* list = check_list(L, 1);
    lua_pop(L, 1);
    lua_pushinteger(L, list->len);
    return 1;
}
// setter
