#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <wayland-util.h>

#include "tile/tile.h"
#include "utils/coreUtils.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "popup.h"

//global variables
struct wl_list clients; /* tiling order */
struct wl_list focusStack;  /* focus order */
struct wl_list stack;   /* stacking z-order */
struct wl_list independents;
struct wl_list layerStack;   /* stacking z-order */
struct wlr_output_layout *output_layout;

void addClientToFocusStack(struct client *c)
{
    if (c) {
        wl_list_insert(&focusStack, &c->flink);
    }
}

void addClientToStack(struct client *c)
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
    unsigned int i, newtags = 0;
    const struct rule *r;
    struct monitor *m; 
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
            i = 0;
            wl_list_for_each(m, &mons, link)
                if (r->monitor == i++)
                    selected_monitor = m;
        }
    }
}

struct client *selected_client()
{
    if (wl_list_length(&focusStack))
    {
        struct client *c = wl_container_of(focusStack.next, c, flink);
        if (!visibleon(c, selected_monitor))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *nextClient()
{
    if (wl_list_length(&focusStack) >= 2)
    {
        struct client *c = wl_container_of(focusStack.next->next, c, flink);
        if (!visibleon(c, selected_monitor))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *prevClient()
{
    if (wl_list_length(&focusStack) >= 2)
    {
        struct client *c = wl_container_of(focusStack.prev, c, flink);
        if (!visibleon(c, selected_monitor))
            return NULL;
        else
            return c;
    } else {
        return NULL;
    }
}

struct client *getClient(int i)
{
    struct client *c;

    if (abs(i) > wl_list_length(&focusStack))
        return NULL;
    if (i == 0)
    {
        c = selected_client();
    } else if (i > 0) {
        struct wl_list *pos = &focusStack;
        while (i > 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        c = wl_container_of(pos, c, flink);
    } else { // i < 0
        struct wl_list *pos = &focusStack;
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
    if (containersInfo.n)
    {
        struct client *c = wl_container_of(stack.next, c, link);
        wl_list_for_each(c, &stack, slink) {
            if (!visibleon(c, selected_monitor))
                continue;
            return c;
        }
    }
    return NULL;
}

struct client *lastClient()
{
    if (containersInfo.n)
    {
        struct client *c = wl_container_of(stack.next, c, link);
        int i = 1;
        wl_list_for_each(c, &stack, slink) {
            if (!visibleon(c, selected_monitor))
                continue;
            if (i > containersInfo.n)
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
        if (visibleon(c, c->mon) && wlr_box_contains_point(&c->geom, x, y)) {
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
    if (m && c) {
        if (c->mon == m) {
            return c->tagset->selTags[0] & m->tagset->selTags[0];
        }
    }
    return false;
}

bool visibleon(struct client *c, struct monitor *m)
{
    // LayerShell based programs are visible on all workspaces
    // TODO: more sophisticated approach with sticky windows needed
    if (m && c) {
        if (c->mon == m) {
            if (c->type == LAYER_SHELL && c->mon == m) {
                return true;
            }

            if (!c->hidden) {
                return c->tagset->selTags[0] & m->tagset->selTags[0];
            }
        }
    }
    return false;
}

bool hiddenon(struct client *c, struct monitor *m)
{
    if (m && c) {
        if (c->mon == m && c->hidden) {
            return c->tagset->selTags[0] & m->tagset->selTags[0];
        }
    }
    return false;
}

bool visible_on_tag(struct client *c, struct monitor *m, size_t focusedTag)
{
    if (m && c) {
        if (c->mon == m) {
            return c->tagset->selTags[0] & position_to_flag(focusedTag);
        }
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
        liftClient(c);

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
    addClientToFocusStack(c);

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

void focusTopClient(struct client *old, bool lift)
{
    struct client *c;
    bool focus = false;

    // focus_stack should not be changed while iterating
    wl_list_for_each(c, &focusStack, flink)
        if (visibleon(c, selected_monitor)) {
            focus = true;
            break;
        }
    if (focus)
        focus_client(old, c, lift);
}

void liftClient(struct client *c)
{
    if (c) {
        wl_list_remove(&c->slink);
        addClientToStack(c);
    }
}
