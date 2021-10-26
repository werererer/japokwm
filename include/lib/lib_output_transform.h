#ifndef LIB_OUTPUT_TRANSFORM_H
#define LIB_OUTPUT_TRANSFORM_H

#include <lauxlib.h>
#include <lua.h>
#include <wayland-server.h>

void lua_load_output_transform(lua_State *L);

// getter
int lib_output_transform_get_normal(lua_State *L);
int lib_output_transform_get_tranform_90(lua_State *L);
int lib_output_transform_get_transform_180(lua_State *L);
int lib_output_transform_get_transform_270(lua_State *L);
int lib_output_transform_get_transform_flipped(lua_State *L);
int lib_output_transform_get_transform_flipped_90(lua_State *L);
int lib_output_transform_get_transform_flipped_180(lua_State *L);
int lib_output_transform_get_transform_flipped_270(lua_State *L);

#endif /* LIB_OUTPUT_TRANSFORM_H */
