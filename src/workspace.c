#include "workspace.h"

#include <assert.h>
#include <string.h>
#include <wayland-server.h>
#include <wlr/util/log.h>

#include "ipc-server.h"
#include "monitor.h"

struct wlr_list workspaces;

struct workspace *create_workspace(const char *name, size_t id, struct layout lt)
{
    struct workspace *ws = malloc(sizeof(struct workspace));
    ws->name = name;
    ws->layout = lt;
    ws->id = id;
    ws->m = NULL;
    return ws;
}

void destroy_workspace(struct workspace *ws)
{
    free(ws);
}

void create_workspaces(struct wlr_list tagNames, struct layout default_layout)
{
    wlr_list_init(&workspaces);
    for (int i = 0; i < tagNames.length; i++) {
        struct workspace *ws = create_workspace(tagNames.items[i], i, default_layout);
        wlr_list_push(&workspaces, ws);
    }
}

void destroy_workspaces()
{
    for (int i = 0; i < number_of_workspaces(); i++)
        destroy_workspace(wlr_list_pop(&workspaces));
    wlr_list_finish(&workspaces);
}

bool is_workspace_occupied(struct workspace *ws)
{
    assert(ws);

    return ws->m ? true : false;
}

int number_of_workspaces()
{
    return workspaces.length;
}

struct workspace *find_next_unoccupied_workspace(struct workspace *ws)
{
    for (size_t i = ws ? ws->id : 0; i < number_of_workspaces(); i++) {
        struct workspace *w = get_workspace(i);
        if (!w)
            break;
        if (!is_workspace_occupied(w))
            return w;
    }
    return NULL;
}

struct workspace *get_workspace(size_t i)
{
    if (i >= workspaces.length)
        return NULL;

    return workspaces.items[i];
}

void workspace_assign_monitor(struct workspace *ws, struct monitor *m)
{
    ws->m = m;
}

void set_selected_layout(struct workspace *ws, struct layout layout)
{
    if (!ws)
        return;

    if (strcmp(ws->name, "") == 0) {
        wlr_log(WLR_ERROR, "ERROR: tag not initialized");
        return;
    }
    ws->layout = layout;
}

void set_next_unoccupied_workspace(struct monitor *m, struct workspace *ws)
{
    struct workspace *w = find_next_unoccupied_workspace(ws);
    set_workspace(m, w);
}

void set_workspace(struct monitor *m, struct workspace *ws)
{
    if (!m || !ws)
        return;
    assert(m->damage != NULL);

    struct container *con;
    wl_list_for_each(con, &sticky_stack, stlink) {
        con->client->ws = ws;
    }

    ipc_event_workspace();

    // unset old workspace
    if (m->ws)
        m->ws->m = NULL;

    m->ws = ws;
    ws->m = m;
    // TODO is wlr_output_damage_whole better? because of floating windows
    root_damage_whole(m->root);
}
