#include "subsurface.h"

#include "container.h"
#include "client.h"
#include "utils/coreUtils.h"

static struct subsurface *create_subsurface(struct wlr_subsurface *wlr_subsurface)
{
    struct subsurface *subsurface = calloc(1, sizeof(struct subsurface));
    subsurface->wlr_subsurface = wlr_subsurface;
    return subsurface;
}

static void destroy_subsurface(struct subsurface *subsurface)
{
    free(subsurface);
}

static void handle_subsurface_commit(struct wl_listener *listener, void *data)
{
    struct subsurface *xdg_subsurface = wl_container_of(listener, xdg_subsurface, commit);
    // TODO: We should damage the subsurface directly and render the damage
    // directly instead of leaving this job to the parent surface. This should
    // save us cpu cycles
    container_damage_part(xdg_subsurface->parent);
}

static void handle_subsurface_destroy(struct wl_listener *listener, void *data)
{
    struct subsurface *subsurface = wl_container_of(listener, subsurface, destroy);
    container_damage_whole(subsurface->parent);
    destroy_subsurface(subsurface);
}

void handle_new_subsurface(struct wl_listener *listener, void *data)
{
    struct client *c = wl_container_of(listener, c, new_subsurface);
    struct wlr_subsurface *subsurface = data;
    struct wlr_surface *surface = subsurface->surface;

    struct subsurface *xdg_subsurface = create_subsurface(subsurface);
    xdg_subsurface->parent = c->con;
    if (!subsurface) {
        return;
    }

    LISTEN(&surface->events.commit, &xdg_subsurface->commit, handle_subsurface_commit);
}

