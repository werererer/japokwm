#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "client.h"

struct client *focustop(struct monitor *m);
void arrange(struct monitor *m, bool reset);
void arrange_client(struct client *c);
void resize(struct client *c, int x, int y, int w, int h, bool interact);
void update_hidden_status();
int thisTiledClientCount();
int tiledClientCount(struct monitor *m);

extern struct containersInfo containersInfo;
#endif
