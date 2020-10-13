#include "tileUtils.h"
#include <coreUtils.h>
#include <julia.h>
#include <string.h>
#include <sys/param.h>

#include "parseConfig.h"

struct cLayoutArr {
  struct wlr_fbox *layout;
  int size;
};

// TODO: move to parseConfigUtils.c
void arrange(Monitor *m, bool reset)
{
    /* Get effective monitor geometry to use for window area */
    m->m = *wlr_output_layout_get_box(output_layout, m->wlr_output);
    m->w = m->m;
    if (m->lt.arrange) {
        Client *c;
        jl_value_t *v = NULL;

        // client count
        int n = 0;
        wl_list_for_each(c, &clients, link)
            if (visibleon(c, m) && !c->isfloating)
                n++;

        // call arrange function
        // if previous layout is different or reset -> reset layout
        if (strcmp(prevLayout.symbol, selMon->lt.symbol) != 0 || reset) {
            prevLayout = selMon->lt;
            jl_value_t *arg1 = jl_box_int64(n);
            v = jl_call1(m->lt.arrange, arg1);
        } else {
            jl_function_t *f = jl_eval_string("Layouts.update");
            jl_value_t *arg1 = jl_box_int64(n);
            v = jl_call1(f, arg1);
        }

        struct cLayoutArr *layoutArr = NULL;
        if (v) {
            layoutArr = jl_unbox_voidpointer(v);

            //place clients
            int i = 0;
            wl_list_for_each(c, &clients, link) {
                if (!visibleon(c, m) || c->isfloating)
                    continue;

                resize(c,
                        layoutArr->layout[i].x*m->w.width,
                        layoutArr->layout[i].y*m->w.height,
                        layoutArr->layout[i].width*m->w.width,
                        layoutArr->layout[i].height*m->w.height, 
                        0);
                i = MIN(i + 1, layoutArr->size-1);
            }
        } else {
            printf("Empty function with symbol: %s\n", m->lt.symbol);
        }
    }
}


void arrangeThis(bool reset)
{
    arrange(selMon, reset);
}

void focusclient(Client *old, Client *c, int lift)
{
    struct wlr_keyboard *kb = wlr_seat_get_keyboard(seat);

    /* Raise client in stacking order if requested */
    if (c && lift) {
        wl_list_remove(&c->slink);
        wl_list_insert(&stack, &c->slink);
    }

    /* Nothing else to do? */
    if (c == old)
        return;

    /* Deactivate old client if focus is changing */
    if (c != old && old) {
#ifdef XWAYLAND
        if (old->type != XDGShell)
            wlr_xwayland_surface_activate(old->surface.xwayland, 0);
        else
#endif
            wlr_xdg_toplevel_set_activated(old->surface.xdg, 0);
    }

    /* Update wlroots' keyboard focus */
    if (!c) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(seat);
        return;
    }

    /* Have a client, so focus its top-level wlr_surface */
    wlr_seat_keyboard_notify_enter(seat, WLR_SURFACE(c),
            kb->keycodes, kb->num_keycodes, &kb->modifiers);

    /* Put the new client atop the focus stack and select its monitor */
    wl_list_remove(&c->flink);
    wl_list_insert(&focus_stack, &c->flink);
    selMon = c->mon;

    /* Activate the new client */
#ifdef XWAYLAND
    if (c->type != XDGShell)
        wlr_xwayland_surface_activate(c->surface.xwayland, 1);
    else
#endif
        wlr_xdg_toplevel_set_activated(c->surface.xdg, 1);
}

Client* focustop(Monitor *m)
{
    Client *c;
    wl_list_for_each(c, &focus_stack, flink)
        if (visibleon(c, m))
            return c;
    return NULL;
}

void setmon(Client *c, Monitor *m, unsigned int newtags)
{
    Monitor *oldmon = c->mon;
    Client *oldsel = selClient();

    if (oldmon == m)
        return;
    c->mon = m;

    /* XXX leave/enter is not optimal but works */
    if (oldmon) {
        wlr_surface_send_leave(WLR_SURFACE(c), oldmon->wlr_output);
        arrange(oldmon, false);
    }
    if (m) {
        /* Make sure window actually overlaps with the monitor */
        applybounds(c, m->m);
        wlr_surface_send_enter(WLR_SURFACE(c), m->wlr_output);
        c->tags = newtags ? newtags : m->tagset[m->seltags]; /* assign tags of target monitor */
        arrange(m, false);
    }
    focusclient(oldsel, focustop(selMon), 1);
}

void
resize(Client *c, int x, int y, int w, int h, int interact)
{
    /*
     * Note that I took some shortcuts here. In a more fleshed-out
     * compositor, you'd wait for the client to prepare a buffer at
     * the new size, then commit any movement that was prepared.
     */
    struct wlr_box bbox = interact ? sgeom : c->mon->w;
    c->geom.x = x;
    c->geom.y = y;
    c->geom.width = w;
    c->geom.height = h;
    applybounds(c, bbox);
    /* wlroots makes this a no-op if size hasn't changed */
#ifdef XWAYLAND
    if (c->type != XDGShell)
        wlr_xwayland_surface_configure(c->surface.xwayland,
                c->geom.x, c->geom.y,
                c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
    else
#endif
        c->resize = wlr_xdg_toplevel_set_size(c->surface.xdg,
                c->geom.width - 2 * c->bw, c->geom.height - 2 * c->bw);
}

bool visibleon(Client *c, Monitor *m)
{
    return c->mon == m && (c->tags & m->tagset[m->seltags]);
}

void updateLayout()
{
    selMon->lt = getConfigLayout("layout");
    arrange(selMon, true);
}
