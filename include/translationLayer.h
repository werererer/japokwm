#ifndef TRANSLATION_LAYER_H
#define TRANSLATION_LAYER_H
#include <lua.h>
#include <lauxlib.h>

struct layout;

// adds a lib table to the table ontop of the lua stack
#define add_table(L, name, functions, idx)\
    do {\
        lua_pushstring(L, name);\
        luaL_newlib(L, functions);\
        lua_settable(L, idx-2);\
    } while(0)

#define set_instance_of(L, name, functions)\
    luaL_newlib(L, functions);\
    luaL_setmetatable(L, name);

#define create_class(L, extra_meta, functions, methods, variable_setter, variable_getter, name)\
    do {\
        luaL_newmetatable(L, name); {\
            luaL_setfuncs(L, meta, 0);\
            lua_pushstring(L, "__metatable");\
            lua_pushstring(L, "access restricted to metatable");\
            lua_settable(L, -3);\
            luaL_setfuncs(L, extra_meta, 0);\
            add_table(L, "setter", variable_setter, -1);\
            add_table(L, "getter", variable_getter, -1);\
            \
            luaL_setfuncs(L, methods, 0);\
            luaL_setfuncs(L, functions, 0);\
        } lua_pop(L, 1);\
    } while(0)

#define create_static_accessor(L, name, functions, static_setter, static_getter)\
    do {\
        lua_createtable(L, 0, 0); {\
            luaL_setfuncs(L, meta, 0);\
            luaL_setfuncs(L, functions, 0);\
            add_table(L, "setter", static_setter, -1);\
            add_table(L, "getter", static_getter, -1);\
            \
            lua_createtable(L, 0, 0);\
            lua_insert(L, -2);\
            lua_setmetatable(L, -2);\
            lua_setglobal(L, name);\
        }\
    } while(0)

#define create_enum(L, variable_getter, name)\
    do {\
        luaL_newmetatable(L, name); {\
            luaL_setfuncs(L, meta, 0);\
            lua_pushstring(L, "__metatable");\
            lua_pushstring(L, "access restricted to metatable");\
            lua_settable(L, -3);\
            \
            lua_pushstring(L, "setter");\
            lua_createtable(L, 0, 0);\
            lua_settable(L, -3);\
            \
            add_table(L, "getter", variable_getter, -1);\
        } lua_pop(L, 1);\
    } while(0)

// lua custom typenames
#define CONFIG_ACTION "japokwm.action"
#define CONFIG_BITSET "japokwm.bitset"
// this is the same as CONFIG_BITSET but also tells that the data field of the
// bitset is of the type tag
#define CONFIG_BITSET_WITH_TAG "japokwm.bitset.tag"
#define CONFIG_BITSET_WITH_CONTAINER "japokwm.bitset.container"
// this is the same as CONFIG_BITSET but lifetime is handle by lua
#define CONFIG_BITSET_GC "japokwm.bitset_gc"
#define CONFIG_COLOR "japokwm.color"
#define CONFIG_CONTAINER "japokwm.container"
#define CONFIG_CONTAINER_PROPERTY "japokwm.container_porperty"
#define CONFIG_CURSOR "japokwm.cursor"
#define CONFIG_CURSOR_MODE "japokwm.cursor_mode"
#define CONFIG_DIRECTION "japokwm.direction"
#define CONFIG_EVENT "japokwm.event"
#define CONFIG_FOCUS_SET "japokwm.focus_set"
#define CONFIG_GEOMETRY "japokwm.geometry"
#define CONFIG_GMP "japokwm.gmp"
#define CONFIG_INFO "japokwm.info"
#define CONFIG_LAYOUT "japokwm.layout"
#define CONFIG_LIST "japokwm.list"
#define CONFIG_LIST2D "japokwm.list2D"
#define CONFIG_LOCAL_OPTIONS "japokwm.local.options"
#define CONFIG_MONITOR "japokwm.monitor"
#define CONFIG_OPTIONS "japokwm.options"
#define CONFIG_OUTPUT_TRANSFORM "japokwm.output_transform"
#define CONFIG_RING_BUFFER "japokwm.ring_buffer"
#define CONFIG_RING_BUFFER_GC "japokwm.ring_buffer_gc"
#define CONFIG_ROOT "japokwm.root"
#define CONFIG_SERVER "japokwm.server"
#define CONFIG_tag "japokwm.tag"

void load_lua_api(lua_State *L);

void init_global_config_variables(lua_State *L);
void init_local_config_variables(lua_State *L, struct layout *lt);

extern const struct luaL_Reg meta[];

int get_lua_value(lua_State *L);
int set_lua_value(lua_State *L);

#endif /* TRANSLATION_LAYER_H */
