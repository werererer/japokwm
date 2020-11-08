#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "client.h"
#include "tile/tileTexture.h"

struct client *focustop(struct monitor *m);
void arrange(struct monitor *m, bool reset);
void resize(struct client *c, int x, int y, int w, int h, bool interact);
void updateHiddenStatus();
int thisTiledClientCount();
int tiledClientCount(struct monitor *m);

extern struct containersInfo containersInfo;
#endif
