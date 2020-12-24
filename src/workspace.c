#include "workspace.h"
#include <wayland-server.h>
#include <string.h>
#include <wlr/util/log.h>
#include "ipc-server.h"
#include "monitor.h"

struct wlr_list workspaces;

struct workspace *create_workspace(char *name, size_t id, struct layout lt)
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

void create_workspaces(struct wlr_list tagNames)
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
    return ws->m ? true : false;
}

int number_of_workspaces()
{
    return workspaces.length;
}

struct workspace *find_next_unoccupied_workspace(struct workspace *ws)
{
    for (int i = ws ? ws->id : 0; i < number_of_workspaces(); i++) {
        struct workspace *w = get_workspace(i);
        if (!is_workspace_occupied(w))
            return w;
    }
    return NULL;
}

struct workspace *get_workspace(size_t i)
{
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
    ipc_event_workspace();

    // unset old workspace
    if (m->ws)
        m->ws->m = NULL;

    m->ws = ws;
    ws->m = m;
}

