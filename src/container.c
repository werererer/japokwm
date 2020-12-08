#include "container.h"
#include "client.h"
#include "parseConfig.h"
#include "server.h"
#include "monitor.h"
#include <wayland-util.h>

static void add_container_to_monitor_stack(struct monitor *m, struct container *con);

struct container *create_container(struct client *c, struct monitor *m)
{
    printf("create_container\n");
    struct container *con = calloc(1, sizeof(struct container));
    con->m = m;
    con->client = c;
    if (con->client->type == LAYER_SHELL)
        wl_list_insert(&m->layer_stack, &con->llink);
    else
        wl_list_insert(&m->containers, &con->mlink);
    wl_list_insert(&c->containers, &con->clink);
    wl_list_insert(&m->focus_stack, &con->flink);
    add_container_to_monitor_stack(m, con);
    return con;
}

void destroy_container(struct container *con)
{
    wl_list_remove(&con->clink);
    wl_list_remove(&con->flink);
    wl_list_remove(&con->slink);
    if (con->client->type == LAYER_SHELL)
        wl_list_remove(&con->llink);
    else
        wl_list_remove(&con->mlink);
    free(con);
}

struct container *selected_container(struct monitor *m)
{
    if (wl_list_empty(&m->focus_stack))
        return NULL;

    struct container *con = wl_container_of(m->focus_stack.next, con, flink);
    if (!visibleon(con, m))
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
        if (!visibleon(con, m))
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
        con = selected_container(m);
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
    if (selected_layout(m->tagset)->containers_info.n <= 0)
        return NULL;

    struct container *con;
    wl_list_for_each(con, &m->stack, slink) {
        if (visibleon(con, m))
            break;
    }
    return con;
}

struct client *last_client(struct monitor *m)
{
    if (selected_layout(m->tagset)->containers_info.n <= 0)
        return NULL;

    struct container *con;
    int i = 1;
    wl_list_for_each(con, &m->stack, slink) {
        if (!visibleon(con, m))
            continue;
        if (i > selected_layout(m->tagset)->containers_info.n)
            return con->client;
        i++;
    }
    return NULL;
}

struct container *xytocontainer(double x, double y)
{
    struct monitor *m = xytomon(x, y);

    struct container *con;
    wl_list_for_each(con, &m->stack, slink) {
        if (visibleon(con, m)
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

void focus_container(struct monitor *m, struct container *con, enum focus_actions a)
{
    struct container *sel = selected_container(m);

    if (!con) {
        /* With no client, all we have left is to clear focus */
        wlr_seat_keyboard_notify_clear_focus(server.seat);
        return;
    }

    if (a == FOCUS_LIFT)
        lift_container(con);

    struct client *c = sel ? sel->client : NULL;
    focus_client(c, con->client);
    /* Put the new client atop the focus stack */
    wl_list_remove(&con->flink);
    wl_list_insert(&m->focus_stack, &con->flink);
}

void lift_container(struct container *con)
{
    if (!con)
        return;
    wl_list_remove(&con->slink);
    add_container_to_monitor_stack(con->m, con);
}

void focus_top_container(struct monitor *m, enum focus_actions a)
{
    wl_list_for_each(m, &mons, link) {
        bool focus = false;

        // focus_stack should not be changed while iterating
        struct container *con;
        wl_list_for_each(con, &m->focus_stack, flink)
            if (visibleon(con, m)) {
                focus = true;
                break;
            }
        if (focus)
            focus_container(m, con, a);
    }
}

bool existon(struct container *con, struct monitor *m)
{
    if (!con || !m)
        return false;
    if (con->m != m)
        return false;

    struct client *c = con->client;

    if (!c)
        return false;

    return c->tagset->selTags[0] & m->tagset->selTags[0];
}

bool hiddenon(struct container *con, struct monitor *m)
{
    if (!con || !m)
        return false;
    if (con->m != m)
        return false;
    if (!con->hidden)
        return false;

    struct client *c = con->client;

    if (!c)
        return false;
    // LayerShell based programs are visible on all workspaces
    if (c->type == LAYER_SHELL)
        return true;

    return c->tagset->selTags[0] & m->tagset->selTags[0];
}

bool visibleon(struct container *con, struct monitor *m)
{
    if (!con || !m)
        return false;
    if (con->m != m)
        return false;
    if (con->hidden)
        return false;

    struct client *c = con->client;

    if (!c)
        return false;
    // LayerShell based programs are visible on all workspaces
    if (c->type == LAYER_SHELL)
        return true;

    return c->tagset->selTags[0] & m->tagset->selTags[0];
}
