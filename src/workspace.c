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

    for (int i = 0; i < 2; i++)
        ws->layout[i] = default_layout;

    push_layout(ws->layout, lt);
    push_layout(ws->layout, lt);

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
        struct layout *dest_prev_lt = &ws->layout[1];
        struct layout *src_lt = &selected_monitor->ws->layout[0];

        if (dest_lt == src_lt)
            continue;

        *dest_lt = copy_layout(src_lt);
        *dest_prev_lt = copy_layout(src_lt);
    }
}

void load_default_layout(lua_State *L, struct workspace *ws)
{
    load_layout(L, ws, default_layout.name);
}

void set_layout(lua_State *L, struct workspace *ws, int layouts_ref)
{
    struct layout *lt = &ws->layout[0];
    if (layouts_ref <= 0)
        return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, layouts_ref);

    // rotate layouts
    if (lua_gettop(L) <= 1) {
        lt->lua_layout_index++;
        if (lt->lua_layout_index > luaL_len(L, -1)) {
            lt->lua_layout_index = 1;
        }
    }

    lua_rawgeti(L, -1, lt->lua_layout_index);
    lua_rawgeti(L, -1, 2);
    const char *layout_name = luaL_checkstring(L, -1);
    lua_pop(L, 3);
    load_layout(L, ws, layout_name);
}


void load_layout(lua_State *L, struct workspace *ws, const char *layout_name)
{
    push_layout(ws->layout, copy_layout(&default_layout));

    struct layout *lt = &ws->layout[0];
    lt->name = layout_name;

    char *config_path = get_config_file("layouts");
    char file[NUM_CHARS] = "";
    strcpy(file, "");
    join_path(file, config_path);
    join_path(file, layout_name);
    join_path(file, "init.lua");
    if (config_path)
        free(config_path);

    if (!file_exists(file))
        return;

    if (luaL_loadfile(L, file)) {
        lua_pop(L, 1);
        return;
    }
    lua_call_safe(L, 0, 0, 0);
}

