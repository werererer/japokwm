#ifndef LIB_EVENT_HANDLER_H
#define LIB_EVENT_HANDLER_H

#include <lua.h>
#include <lauxlib.h>

struct event_handler;

void create_lua_event_handler(struct event_handler *event_handler);
void lua_load_events();
void lua_init_events(struct event_handler *event_handler);

int lib_add_listener(lua_State *L);

#endif /* LIB_EVENT_HANDLER_H */
