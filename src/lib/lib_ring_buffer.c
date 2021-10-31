#include "lib/lib_ring_buffer.h"

#include "translationLayer.h"
#include "ring_buffer.h"

static const struct luaL_Reg ring_buffer_meta_gc[] =
{
    {"__gc", lib_ring_buffer_gc},
    {NULL, NULL},
};

static const struct luaL_Reg ring_buffer_meta[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg ring_buffer_f[] =
{
    {"new", lib_ring_buffer_new},
    {NULL, NULL},
};

static const struct luaL_Reg ring_buffer_m[] =
{
    {"get", lib_ring_buffer_get},
    {"next", lib_ring_buffer_next},
    {"prev", lib_ring_buffer_prev},
    {NULL, NULL},
};

static const struct luaL_Reg ring_buffer_setter[] =
{
    {NULL, NULL},
};

static const struct luaL_Reg ring_buffer_getter[] =
{
    {NULL, NULL},
};

void lua_load_ring_buffer(lua_State *L)
{
    create_class(L,
            ring_buffer_meta,
            ring_buffer_f,
            ring_buffer_m,
            ring_buffer_setter,
            ring_buffer_getter,
            CONFIG_RING_BUFFER);
    create_class(L,
            ring_buffer_meta_gc,
            ring_buffer_f,
            ring_buffer_m,
            ring_buffer_setter,
            ring_buffer_getter,
            CONFIG_RING_BUFFER_GC);

    luaL_newlib(L, ring_buffer_f);
    lua_setglobal(L, "Ring");
}

struct ring_buffer *check_ring_buffer(lua_State *L, int narg)
{
    void **ud = NULL;
    if (luaL_testudata(L, narg, CONFIG_RING_BUFFER)) {
         ud = luaL_checkudata(L, narg, CONFIG_RING_BUFFER);
    } else if (luaL_testudata(L, narg, CONFIG_RING_BUFFER_GC)) {
         ud = luaL_checkudata(L, narg, CONFIG_RING_BUFFER_GC);
    }
    luaL_argcheck(L, ud != NULL, narg, "`ring_buffer' expected");
    return *(struct ring_buffer **)ud;
}

void create_lua_ring_buffer(lua_State *L, struct ring_buffer *ring_buffer)
{
    struct ring_buffer **user_ring_buffer = lua_newuserdata(L, sizeof(struct ring_buffer *));
    *user_ring_buffer = ring_buffer;

    luaL_setmetatable(L, CONFIG_RING_BUFFER);
}

void create_lua_ring_buffer_gc(lua_State *L, struct ring_buffer *ring_buffer)
{
    struct ring_buffer **user_ring_buffer = lua_newuserdata(L, sizeof(struct ring_buffer *));
    *user_ring_buffer = ring_buffer;

    luaL_setmetatable(L, CONFIG_RING_BUFFER_GC);
}

int lib_ring_buffer_gc(lua_State *L)
{
    struct ring_buffer *ring_buffer = check_ring_buffer(L, 1);
    lua_pop(L, 1);
    destroy_ring_buffer(ring_buffer);
    return 0;
}
// functions
int lib_ring_buffer_new(lua_State *L)
{
    GPtrArray *names = g_ptr_array_new();
    int len = luaL_len(L, 1);
    for (int i = 1; i <= len; i++) {
        lua_rawgeti(L, 1, i);
        const char *content = luaL_checkstring(L, -1);
        g_ptr_array_add(names, strdup(content));
        lua_pop(L, 1);
    }

    struct ring_buffer *ring_buffer = create_ring_buffer();
    ring_buffer_set_names(ring_buffer, names);
    g_ptr_array_unref(names);

    create_lua_ring_buffer_gc(L, ring_buffer);
    return 1;
}

// methods
int lib_ring_buffer_get(lua_State *L)
{
    struct ring_buffer *ring_buffer = check_ring_buffer(L, 1);

    const char *content = ring_buffer_get(ring_buffer);
    lua_pushstring(L, content);
    return 1;
}

int lib_ring_buffer_prev(lua_State *L)
{
    struct ring_buffer *ring_buffer = check_ring_buffer(L, 1);

    ring_buffer_rotate(ring_buffer, -1);
    const char *content = ring_buffer_get(ring_buffer);
    lua_pushstring(L, content);
    return 1;
}

int lib_ring_buffer_next(lua_State *L)
{
    struct ring_buffer *ring_buffer = check_ring_buffer(L, 1);
    for (int i = 0; i < ring_buffer->names->len; i++) {
        const char *name = g_ptr_array_index(ring_buffer->names, i);
        printf("name: %s\n", name);
    }
    ring_buffer_rotate(ring_buffer, 1);
    const char *content = ring_buffer_get(ring_buffer);
    lua_pushstring(L, content);
    return 1;
}
// getter
// setter
