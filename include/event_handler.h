#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <lua.h>
#include <stdlib.h>
#include <glib.h>

struct container;

struct event_handler {
    GPtrArray *on_start_func_refs;
    GPtrArray *on_unfocus_func_refs;
    GPtrArray *on_focus_func_refs;
    GPtrArray *on_update_func_refs;
    GPtrArray *on_create_container_func_refs;
};

struct event_handler *create_event_handler();
void destroy_event_handler(struct event_handler *event_handler);

GPtrArray *event_name_to_signal(struct event_handler *event_handler,
        const char *event);

/*
 * int n refers to the affected container position in container stack
 */
void call_create_container_function(struct event_handler *ev, int n);
void call_on_focus_function(struct event_handler *ev, struct container *con);
void call_on_unfocus_function(struct event_handler *ev, struct container *con);
void call_on_start_function(struct event_handler *ev);
void call_update_function(struct event_handler *ev, int n);

#endif /* EVENT_HANDLER_H */
