#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <lua.h>

struct event_handler {
    int update_func_ref;
    int create_container_func_ref;
    int on_start_func_ref;
    int on_focus_func_ref;
};

struct event_handler get_default_event_handler();

/*
 * int n refers to the affected container position in container stack
 */
void call_create_container_function(struct event_handler *ev, int n);
void call_on_focus_function(struct event_handler *ev, int n);
void call_on_start_function(struct event_handler *ev);
void call_update_function(struct event_handler *ev, int n);

#endif /* EVENT_HANDLER_H */
