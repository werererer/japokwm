#ifndef TILEUTILS
#define TILEUTILS
#include "coreUtils.h"
#include "client.h"
#include <julia.h>

extern Layout layout;

Client *focustop(Monitor *m);
bool visibleon(Client *c, Monitor *m);
jl_value_t* getWlrBox(struct wlr_box w);
void arrange(Monitor *m);
void focusclient(Client *old, Client *c, int lift);
void setmon(Client *c, Monitor *m, unsigned int newtags);
void updateLayout();
#endif
