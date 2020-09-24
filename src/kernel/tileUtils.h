#ifndef TILEUTILS
#define TILEUTILS
#include "coreUtils.h"
#include "client.h"
#include <julia.h>

void arrange(Monitor *m);
void focusclient(Client *old, Client *c, int lift);
Client *focustop(Monitor *m);
void setmon(Client *c, Monitor *m, unsigned int newtags);
#endif
