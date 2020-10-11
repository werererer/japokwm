#include "tileUtils.h"
#include <julia.h>
#include <parseConfigUtils.h>

Layout layout;

// TODO: move to parseConfigUtils.c
void
arrange(Monitor *m)
{
    /* Get effective monitor geometry to use for window area */
    m->m = *wlr_output_layout_get_box(output_layout, m->wlr_output);
    m->w = m->m;
    if (m->lt.arrange) {
        jl_value_t *arg1 = toJlMonitor("Layouts.Monitor", selmon);
        jl_call1(m->lt.arrange, arg1);
    }
    /* XXX recheck pointer focus here... or in resize()? */
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
    selmon = c->mon;
    printf("%i selmon: %i\n", __LINE__, selmon == NULL);

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
        arrange(oldmon);
    }
    if (m) {
        /* Make sure window actually overlaps with the monitor */
        applybounds(c, m->m);
        wlr_surface_send_enter(WLR_SURFACE(c), m->wlr_output);
        c->tags = newtags ? newtags : m->tagset[m->seltags]; /* assign tags of target monitor */
        arrange(m);
    }
    focusclient(oldsel, focustop(selmon), 1);
}

bool visibleon(Client *c, Monitor *m)
{
    return c->mon == m && (c->tags & m->tagset[m->seltags]);
}

void updateLayout()
{
    printf("%i selmon: %i\n", __LINE__, selmon == NULL);
    if (selmon) {
        selmon->lt = getConfigLayout("layout");
    } else {
        printf("empty \n");
    }
}

