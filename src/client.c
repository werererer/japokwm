#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "tile/tile.h"
#include "tile/tileTexture.h"
#include "utils/coreUtils.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"

//global variables
struct wl_list clients; /* tiling order */
struct wl_list focus_stack;  /* focus order */
struct wl_list stack;   /* stacking z-order */
struct wl_list independents;
struct wl_list layerstack;   /* stacking z-order */
struct wlr_output_layout *output_layout;

void add_client_to_focusstack(struct client *c)
{
    if (c) {
        wl_list_insert(&focus_stack, &c->flink);
    }
}

void add_client_to_stack(struct client *c)
{
    if (c) {
        if (c->floating) {
            wl_list_insert(&stack, &c->slink);
        } else {
            /* Insert client after the last floating client */
            struct client *client;
            wl_list_for_each(client, &stack, slink) {
                if (!client->floating)
                    break;
            }
            wl_list_insert(client->slink.prev, &c->slink);
        }
    }
}

void applybounds(struct client *c, struct wlr_box bbox)
{
    /* set minimum possible */
    c->geom.width = MAX(30, c->geom.width);
    c->geom.height = MAX(30, c->geom.height);

    if (c->geom.x >= bbox.x + bbox.width)
        c->geom.x = bbox.x + bbox.width - c->geom.width;
    if (c->geom.y >= bbox.y + bbox.height)
        c->geom.y = bbox.y + bbox.height - c->geom.height;
    if (c->geom.x + c->geom.width + 2 * c->bw <= bbox.x)
        c->geom.x = bbox.x;
    if (c->geom.y + c->geom.height + 2 * c->bw <= bbox.y)
        c->geom.y = bbox.y;
}

void applyrules(struct client *c)
{
    const char *appid, *title;
    unsigned int newtags = 0;
    const struct rule *r;
    /* rule matching */
    c->floating = false;
    switch (c->type) {
        case XDG_SHELL:
            appid = c->surface.xdg->toplevel->app_id;
            title = c->surface.xdg->toplevel->title;
            break;
        case LAYER_SHELL:
            appid = "test";
            title = "test";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            appid = c->surface.xwayland->class;
            title = c->surface.xwayland->title;
            break;
    }
    if (!appid)
        appid = "broken";
    if (!title)
        title = "broken";

    for (r = rules; r < END(rules); r++) {
        if ((!r->title || strstr(title, r->title))
                && (!r->id || strstr(appid, r->id))) {
            c->floating = r->floating;
            newtags |= r->tags;
        }
    }
}

struct client *selected_client()
{
    if (wl_list_length(&focus_stack))
    {
        struct client *c = wl_container_of(focus_stack.next, c, flink);
        if (!visibleon(c, selected_monitor->tagset))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *next_client()
{
    if (wl_list_length(&focus_stack) >= 2)
    {
        struct client *c = wl_container_of(focus_stack.next->next, c, flink);
        if (!visibleon(c, selected_monitor->tagset))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *prev_client()
{
    if (wl_list_length(&focus_stack) >= 2)
    {
        struct client *c = wl_container_of(focus_stack.prev, c, flink);
        if (!visibleon(c, selected_monitor->tagset))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *get_client(int i)
{
    struct client *c;

    if (abs(i) > wl_list_length(&focus_stack))
        return NULL;
    if (i == 0)
    {
        c = selected_client();
    } else if (i > 0) {
        struct wl_list *pos = &focus_stack;
        while (i > 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        c = wl_container_of(pos, c, flink);
    } else { // i < 0
        struct wl_list *pos = &focus_stack;
        while (i < 0) {
            pos = pos->prev;
            i++;
        }
        c = wl_container_of(pos, c, flink);
    }
    return c;
}

struct client *firstClient()
{
    if (containers_info.n)
    {
        struct client *c = wl_container_of(stack.next, c, link);
        wl_list_for_each(c, &stack, slink) {
            if (!visibleon(c, selected_monitor->tagset))
                continue;
            return c;
        }
    }
    return NULL;
}

struct client *last_client()
{
    if (containers_info.n)
    {
        struct client *c = wl_container_of(stack.next, c, link);
        int i = 1;
        wl_list_for_each(c, &stack, slink) {
            if (!visibleon(c, selected_monitor->tagset))
                continue;
            if (i > containers_info.n)
                return c;
            i++;
        }
    }
    return NULL;
}

struct client *xytoclient(double x, double y)
{
    /* Find the topmost visible client (if any) at point (x, y), including
     * borders. This relies on stack being ordered from top to bottom. */
    struct client *c;
    wl_list_for_each(c, &stack, slink) {
        struct wlr_box box = get_absolute_box(selected_monitor->m, c->geom);
        if (visibleon(c, selected_monitor->tagset)
                && wlr_box_contains_point(&box, x, y)) {
            return c;
        }
    }
    return NULL;
}

struct wlr_surface *get_wlrsurface(struct client *c)
{
    if (!c)
        return NULL;
    switch (c->type) {
        case XDG_SHELL:
            return c->surface.xdg->surface;
            break;
        case LAYER_SHELL:
            return c->surface.layer->surface;
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            return c->surface.xwayland->surface;
        default:
            printf("wlr_surface is not supported: \n");
            return NULL;
    }
}

bool existon(struct client *c, struct monitor *m)
{
    if (c) {
        return c->tagset->selTags[0] & m->tagset->selTags[0];
    }
    return false;
}

bool visibleon(struct client *c, struct tagset *tagset)
{
    // LayerShell based programs are visible on all workspaces
    if (c && tagset) {
        if (c->type == LAYER_SHELL) {
            return true;
        }

        if (!c->hidden) {
            return c->tagset->selTags[0] & tagset->selTags[0];
        }
    }
    return false;
}

bool hiddenon(struct client *c, struct monitor *m)
{
    if (c) {
        if (c->hidden) {
            return c->tagset->selTags[0] & m->tagset->selTags[0];
        }
    }
    return false;
}

bool visible_on_tag(struct client *c, struct monitor *m, size_t focusedTag)
{
    if (c) {
        return c->tagset->selTags[0] & position_to_flag(focusedTag);
    }
    return false;
}

bool client_visible_on_tag(struct client *c, size_t focusedTag)
{
    if (c) {
        return c->tagset->selTags[0] & position_to_flag(focusedTag);
    }
    return false;
}

static void unfocusClient(struct client *c)
{
    if (c) {
        switch (c->type) {
            case XDG_SHELL:
                wlr_xdg_toplevel_set_activated(c->surface.xdg, false);
                break;
            case X11_MANAGED:
            case X11_UNMANAGED:
                wlr_xwayland_surface_activate(c->surface.xwayland, false);
                break;
            default:
                break;
        }
    }
}

void focus_client(struct client *old, struct client *c, bool lift)
{
    if (old == c)
        return;

    struct wlr_keyboard *kb = wlr_seat_get_keyboard(server.seat);

    if (lift)
        lift_client(c);

    unfocusClient(old);
    /* Update wlroots' keyboard focus */
    if (!c) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(server.seat);
        return;
    }

    /* Have a client, so focus its top-level wlr_surface */
    wlr_seat_keyboard_notify_enter(server.seat, get_wlrsurface(c), kb->keycodes,
            kb->num_keycodes, &kb->modifiers);

    /* Put the new client atop the focus stack */
    wl_list_remove(&c->flink);
    add_client_to_focusstack(c);

    /* Activate the new client */
    switch (c->type) {
        case XDG_SHELL:
            wlr_xdg_toplevel_set_activated(c->surface.xdg, true);
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            wlr_xwayland_surface_activate(c->surface.xwayland, true);
            break;
        default:
            break;
    }
}

void focus_top_client(struct client *old, bool lift)
{
    struct client *c;
    bool focus = false;

    // focus_stack should not be changed while iterating
    wl_list_for_each(c, &focus_stack, flink)
        if (visibleon(c, selected_monitor->tagset)) {
            focus = true;
            break;
        }
    if (focus)
        focus_client(old, c, lift);
}

void lift_client(struct client *c)
{
    if (c) {
        wl_list_remove(&c->slink);
        add_client_to_stack(c);
    }
}
