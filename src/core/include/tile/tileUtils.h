#ifndef TILEUTILS
#define TILEUTILS
#include "coreUtils.h"
#include "client.h"
#include <julia.h>
#include "tile/tileTexture.h"

Client *focustop(Monitor *m);
jl_value_t* getWlrBox(struct wlr_box w);
void arrange(Monitor *m, bool reset);
void arrangeThis(bool reset);
void focusclient(Client *old, Client *c, bool lift);
void setmon(Client *c, Monitor *m, unsigned int newtags);
void resize(Client *c, int x, int y, int w, int h, int interact);
void updateLayout();
int thisTiledClientCount();
int tiledClientCount(Monitor *m);
int clientPos();
#endif
