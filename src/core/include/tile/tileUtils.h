#ifndef TILEUTILS
#define TILEUTILS
#include "utils/coreUtils.h"
#include "client.h"
#include <julia.h>
#include "tile/tileTexture.h"

struct client *focustop(struct monitor *m);
jl_value_t* getWlrBox(struct wlr_box w);
void arrange(struct monitor *m, bool reset);
void arrangeThis(bool reset);
void resize(struct client *c, int x, int y, int w, int h, bool interact);
void updateLayout();
int thisTiledClientCount();
int tiledClientCount(struct monitor *m);
int clientPos();

// this exposes the overlay variable to julia
void setOverlay(bool ol);
bool getOverlay();
extern bool overlay;
#endif
