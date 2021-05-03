#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <wayland-server.h>


void set_primary_selection(struct wl_listener *listener, void *data);
void set_selection(struct wl_listener *listener, void *data);

extern struct wl_listener request_set_psel;
extern struct wl_listener request_set_sel;
#endif /* CLIPBOARD_H */
