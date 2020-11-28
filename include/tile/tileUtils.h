#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "client.h"

struct client *focustop(struct monitor *m);
void arrange(struct monitor *m, bool reset);
void arrange_client(struct client *c, int i);
void resize(struct client *c, int x, int y, int w, int h, bool interact);
void update_hidden_status(struct monitor *m);
int thisTiledClientCount();
int tiled_client_count(struct monitor *m);

struct wlr_box get_absolute_box(struct wlr_box box, struct wlr_fbox b);
struct wlr_fbox get_relative_box(struct wlr_box box, struct wlr_box b);

extern struct containers_info containers_info;
#endif
