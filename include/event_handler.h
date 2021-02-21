#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <lua.h>

struct event_handler {
    int update_func_ref;
    int create_container_func_ref;
};

struct event_handler get_default_event_handler();

void call_update_function(struct event_handler *ev, int n);
void call_create_container_function(struct event_handler *ev, int n);

#endif /* EVENT_HANDLER_H */
