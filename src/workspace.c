#include "workspace.h"

#include <assert.h>
#include <string.h>
#include <wayland-server.h>
#include <wlr/util/log.h>

#include "ipc-server.h"
#include "monitor.h"

static struct wlr_list workspaces;

struct workspace *create_workspace(const char *name, size_t id, struct layout lt)
{
    struct workspace *ws = malloc(sizeof(struct workspace));
    ws->name = name;

    ws->layout[0] = lt;

    lua_get_basic_layout();
    ws->layout[0].lua_layout_copy_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_createtable(L, 0, 0);
    ws->layout[0].lua_layout_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    lua_get_basic_layout();
    ws->layout[0].lua_layout_master_copy_data_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ws->id = id;
    ws->m = NULL;
    return ws;
}

void destroy_workspace(struct workspace *ws)
{
    free(ws);
}

void init_workspaces()
{
    wlr_list_init(&workspaces);
}

void create_workspaces(struct wlr_list tagNames, struct layout default_layout)
{
    for (int i = 0; i < tagNames.length; i++) {
        struct workspace *ws = create_workspace(tagNames.items[i], i, default_layout);
        wlr_list_push(&workspaces, ws);
    }
}

void destroy_workspaces()
{
    for (int i = 0; i < workspace_count(); i++)
        destroy_workspace(wlr_list_pop(&workspaces));
    wlr_list_finish(&workspaces);
}

bool is_workspace_occupied(struct workspace *ws)
{
    assert(ws);

    return ws->m ? true : false;
}

bool workspace_has_clients(struct workspace *ws)
{
    if (!ws)
        return 0;

    int count = 0;

    struct client *c;
    wl_list_for_each(c, &clients, link)
        if (c->ws == ws)
            count++;

    return count > 0;
}

int workspace_count()
{
    return workspaces.length;
}

struct workspace *find_next_unoccupied_workspace(struct workspace *ws)
{
    for (size_t i = ws ? ws->id : 0; i < workspace_count(); i++) {
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
    printf("set selected layout\n");
    if (!ws)
        return;

    if (strcmp(ws->name, "") == 0) {
        wlr_log(WLR_ERROR, "ERROR: tag not initialized");
        return;
    }
    push_layout(ws->layout, layout);
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

void copy_layout_from_selected_workspace()
{
    for (int i = 0; i < workspaces.length; i++) {
        struct workspace *ws = workspaces.items[i];
        struct layout *dest_lt = &ws->layout[0];
        struct layout *src_lt = &selected_monitor->ws->layout[0];

        if (dest_lt == src_lt)
            continue;

        copy_layout(dest_lt, src_lt);
    }
}
