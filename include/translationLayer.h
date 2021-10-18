#ifndef TRANSLATION_LAYER_H
#define TRANSLATION_LAYER_H
#include <lua.h>
#include <lauxlib.h>

struct workspace;

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

#define create_class(extra_meta, functions, methods, variable_setter, variable_getter, name)\
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

// lua custom typenames
#define CONFIG_COLOR "japokwm.color"
#define CONFIG_CONTAINER "japokwm.container"
#define CONFIG_DIRECTION "japokwm.direction"
#define CONFIG_EVENT "japokwm.event"
#define CONFIG_GEOMETRY "japokwm.geometry"
#define CONFIG_LAYOUT "japokwm.layout"
#define CONFIG_LIST "japokwm.list"
#define CONFIG_LIST2D "japokwm.list2D"
#define CONFIG_LOCAL_OPTIONS "japokwm.local.options"
#define CONFIG_OPTIONS "japokwm.options"
#define CONFIG_SERVER "japokwm.server"
#define CONFIG_WORKSPACE "japokwm.workspace"

void load_lua_api(lua_State *L);

void init_global_config_variables(lua_State *L);
void init_local_config_variables(lua_State *L, struct workspace *ws);

extern const struct luaL_Reg meta[];

int get_lua_value(lua_State *L);
int set_lua_value(lua_State *L);

#endif /* TRANSLATION_LAYER_H */
