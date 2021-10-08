#ifndef TRANSLATION_LAYER_H
#define TRANSLATION_LAYER_H
#include <lua.h>
#include <lauxlib.h>

// adds a lib table to the table ontop of the lua stack
#define add_table(L, name, functions, idx)\
    do {\
        lua_pushstring(L, name);\
        luaL_newlib(L, functions);\
        lua_settable(L, idx-2);\
    } while(0)

// adds a metatable to the table on top of the lua stack
#define add_meta_table(methods, variable_setter, variable_getter, name)\
    do {\
        create_class(methods, variable_setter, variable_getter, name);\
        luaL_setmetatable(L, name);\
    } while(0)

#define create_class(methods, variable_setter, variable_getter, name)\
    do {\
        luaL_newmetatable(L, name); {\
            luaL_setfuncs(L, meta, 0);\
            add_table(L, "setter", variable_setter, -1);\
            add_table(L, "getter", variable_getter, -1);\
            \
            luaL_setfuncs(L, methods, 0);\
            \
            lua_pushstring(L, "__index");\
            lua_pushvalue(L, -2);  /* pushes the metatable */\
            lua_settable(L, -3);  /* metatable.__index = metatable */\
        } lua_pop(L, 1);\
    } while(0)


void load_lua_api(lua_State *L);

extern const struct luaL_Reg meta[];

#endif /* TRANSLATION_LAYER_H */
