#include "container.h"
#include "client.h"
#include "parseConfig.h"
#include "server.h"
#include "monitor.h"
#include <wayland-util.h>

struct containers_info containers_info;

static void add_container_to_monitor_stack(struct monitor *m, struct container *con);

struct container *create_container(struct client *c, struct monitor *m)
{
    printf("create_container\n");
    struct container *con = calloc(1, sizeof(struct container));
    con->m = m;
    con->client = c;
    wl_list_insert(&c->containers, &con->clink);
    wl_list_insert(&m->containers, &con->mlink);
    wl_list_insert(&m->focus_stack, &con->flink);
    add_container_to_monitor_stack(m, con);
    return con;
}

void destroy_container(struct container *con)
{
    wl_list_remove(&con->clink);
    wl_list_remove(&con->mlink);
    wl_list_remove(&con->flink);
    wl_list_remove(&con->slink);
    free(con);
}

struct container *selected_container()
{
    if (wl_list_empty(&selected_monitor->focus_stack))
        return NULL;

    struct container *con =
        wl_container_of(selected_monitor->focus_stack.next, con, flink);
    if (!visibleon(con->client, selected_monitor->tagset))
        return NULL;
    else
        return con;
}

struct container *next_container(struct monitor *m)
{
    if (wl_list_length(&m->focus_stack) >= 2)
    {
        struct container *con =
            wl_container_of(m->focus_stack.next->next, con, flink);
        if (!visibleon(con->client, selected_monitor->tagset))
            return NULL;
        else
            return con;
    } else {
        return NULL;
    }
}

struct container *get_container(struct monitor *m, int i)
{
    struct container *con;

    if (abs(i) > wl_list_length(&m->focus_stack))
        return NULL;
    if (i == 0)
    {
        con = selected_container();
    } else if (i > 0) {
        struct wl_list *pos = &m->focus_stack;
        while (i > 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        con = wl_container_of(pos, con, flink);
    } else { // i < 0
        struct wl_list *pos = &m->focus_stack;
        while (i < 0) {
            pos = pos->prev;
            i++;
        }
        con = wl_container_of(pos, con, flink);
    }
    return con;
}

struct container *first_container(struct monitor *m)
{
    if (containers_info.n)
    {
        struct container *con;
        wl_list_for_each(con, &m->stack, slink) {
            if (!visibleon(con->client, selected_monitor->tagset))
                continue;
            return con;
        }
    }
    return NULL;
}

struct client *last_client(struct monitor *m)
{
    if (containers_info.n)
    {
        struct container *con;
        int i = 1;
        wl_list_for_each(con, &m->stack, slink) {
            if (!visibleon(con->client, selected_monitor->tagset))
                continue;
            if (i > containers_info.n)
                return con->client;
            i++;
        }
    }
    return NULL;
}

struct container *xytocontainer(double x, double y)
{
    struct monitor *m = xytomon(x, y);

    struct container *con;
    wl_list_for_each(con, &m->stack, slink) {
        if (visibleon(con->client, selected_monitor->tagset)
                && wlr_box_contains_point(&con->geom, x, y)) {
            return con;
        }
    }
    return NULL;
}

static void add_container_to_monitor_stack(struct monitor *m, struct container *con)
{
    if (con) {
        if (con->floating) {
            wl_list_insert(&m->stack, &con->slink);
        } else {
            /* Insert container after the last floating container */
            struct container *c;
            wl_list_for_each(c, &m->stack, slink) {
                if (!c->floating)
                    break;
            }

            if (!wl_list_empty(&m->stack)) {
                wl_list_insert(&c->slink, &con->slink);
            } else {
                wl_list_insert(&m->stack, &con->slink);
            }
        }
    }
}

void add_container_to_monitor(struct monitor *m, struct container *con)
{
    printf("add_container_to_monitor\n");
    if (m && con) {
        wl_list_insert(&m->containers, &con->mlink);
        add_container_to_monitor_stack(m, con);
        wl_list_insert(&m->focus_stack, &con->flink);
    }
}

struct wlr_box get_absolute_box(struct wlr_box box, struct wlr_fbox b)
{
    struct wlr_box w;
    w.x = b.x * box.width + box.x;
    w.y = b.y * box.height + box.y;
    w.width = box.width * b.width;
    w.height = box.height * b.height;
    return w;
}

struct wlr_fbox get_relative_box(struct wlr_box box, struct wlr_box b)
{
    struct wlr_fbox w;
    w.x = (float)box.x / b.width;
    w.y = (float)box.y / b.height;
    w.width = (float)box.width / b.width;
    w.height = (float)box.height / b.height;
    return w;
}

void applybounds(struct container *con, struct wlr_box bbox)
{
    /* set minimum possible */
    con->geom.width = MAX(30, con->geom.width);
    con->geom.height = MAX(30, con->geom.height);

    if (con->geom.x >= bbox.x + bbox.width)
        con->geom.x = bbox.x + bbox.width - con->geom.width;
    if (con->geom.y >= bbox.y + bbox.height)
        con->geom.y = bbox.y + bbox.height - con->geom.height;
    if (con->geom.x + con->geom.width + 2 * con->client->bw <= bbox.x)
        con->geom.x = bbox.x;
    if (con->geom.y + con->geom.height + 2 * con->client->bw <= bbox.y)
        con->geom.y = bbox.y;
}

void applyrules(struct container *con)
{
    const char *appid, *title;
    unsigned int newtags = 0;
    const struct rule *r;
    /* rule matching */
    con->floating = false;
    switch (con->client->type) {
        case XDG_SHELL:
            appid = con->client->surface.xdg->toplevel->app_id;
            title = con->client->surface.xdg->toplevel->title;
            break;
        case LAYER_SHELL:
            appid = "test";
            title = "test";
            break;
        case X11_MANAGED:
        case X11_UNMANAGED:
            appid = con->client->surface.xwayland->class;
            title = con->client->surface.xwayland->title;
            break;
    }
    if (!appid)
        appid = "broken";
    if (!title)
        title = "broken";

    for (r = rules; r < END(rules); r++) {
        if ((!r->title || strstr(title, r->title))
                && (!r->id || strstr(appid, r->id))) {
            con->floating = r->floating;
            newtags |= r->tags;
        }
    }
}

void focus_container(struct monitor *m, struct container *con, bool lift)
{
    printf("focus container\n");
    printf("geom: x: %i\n", con->geom.x);
    printf("geom: y: %i\n", con->geom.y);
    printf("geom: width: %i\n", con->geom.width);
    printf("geom: height: %i\n", con->geom.height);
    /* if (con == selected_container()) */
    /*     return; */

    if (lift)
        lift_container(con);

    /* Update wlroots' keyboard focus */
    if (!con) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(server.seat);
        return;
    }
    focus_client(selected_container()->client, con->client);
    /* Put the new client atop the focus stack */
    wl_list_remove(&con->flink);
    wl_list_insert(&m->focus_stack, &con->flink);
}

void lift_container(struct container *con)
{
    if (con) {
        wl_list_remove(&con->slink);
        add_container_to_monitor_stack(con->m, con);
    }
}

void focus_top_container(bool lift)
{
    struct monitor *m;
    wl_list_for_each(m, &mons, link) {
        bool focus = false;

        // focus_stack should not be changed while iterating
        struct container *con;
        wl_list_for_each(con, &m->focus_stack, flink)
            if (visibleon(con->client, selected_monitor->tagset)) {
                focus = true;
                break;
            }
        if (focus)
            focus_container(m, con, lift);
    }
}
