#ifndef MOUSE_H
#define MOUSE_H

extern struct client *grabc;
extern int grabcx, grabcy; /* client-relative */

void motionrelative(struct wl_listener *listener, void *data);

#endif /* MOUSE_H */
