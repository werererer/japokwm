#ifndef LIB_RING_BUFFER_H
#define LIB_RING_BUFFER_H

#include <lua.h>
#include <lauxlib.h>
#include <glib.h>
#include <wayland-server-core.h>

void lua_load_ring_buffer(lua_State *L);
struct ring_buffer *check_ring_buffer(lua_State *L, int narg);
void create_lua_ring_buffer(lua_State *L, struct ring_buffer *ring_buffer);

int lib_ring_buffer_gc(lua_State *L);

// functions
int lib_ring_buffer_new(lua_State *L);
// methods
int lib_ring_buffer_get(lua_State *L);
int lib_ring_buffer_prev(lua_State *L);
int lib_ring_buffer_next(lua_State *L);
// getter
// setter

#endif /* LIB_RING_BUFFER_H */
