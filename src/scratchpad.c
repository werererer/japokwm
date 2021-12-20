#include "scratchpad.h"

#include "client.h"
#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "tag.h"
#include "tagset.h"

// TODO rewrite this function so it is easier to read
void move_to_scratchpad(struct container *con, int position)
{
    if (!con)
        return;
    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }

    struct monitor *m = container_get_monitor(con);
    // TODO: try to remove this
    container_set_floating(con, container_fix_position, false);

    con->ws_id = INVALID_WORKSPACE_ID;
    con->on_scratchpad = true;

    if (server.scratchpad->len== 0) {
        g_ptr_array_add(server.scratchpad, con);
    } else {
        int new_position = relative_index_to_absolute_index(0,
                position, server.scratchpad->len+1);
        g_ptr_array_insert(server.scratchpad, new_position, con);
    }

    struct tag *ws = monitor_get_active_workspace(m);
    tagset_reload(ws);

    container_damage_whole(con);
    arrange();
    focus_most_recent_container();
}

void remove_container_from_scratchpad(struct container *con)
{
    g_ptr_array_remove(server.scratchpad, con);
    con->on_scratchpad = false;
}

static void hide_container(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    struct container *sel = monitor_get_focused_container(m);

    if (!sel->on_scratchpad) {
        focus_container(con);
        lift_container(con);
        return;
    }

    move_to_scratchpad(con, -1);
}

static void show_container(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *ws = monitor_get_active_workspace(m);

    container_set_hidden(con, false);
    container_set_workspace_id(con, ws->id);
    container_set_floating(con, container_fix_position, true);
    struct wlr_box center_box = get_center_box(m->geom);
    container_set_current_geom(con, &center_box);

    focus_container(con);
    lift_container(con);
    arrange();
}

void show_scratchpad()
{
    if (server.scratchpad->len <= 0)
        return;

    struct container *con = g_ptr_array_index(server.scratchpad, 0);
    struct monitor *m = server_get_selected_monitor();
    struct tag *ws = monitor_get_active_workspace(m);
    bool visible_on_other_workspace = !container_get_hidden(con) && ws->id != con->ws_id;
    if (visible_on_other_workspace) {
        container_set_workspace_id(con, ws->id);
        container_set_floating(con, container_fix_position, true);
        struct wlr_box center_box = get_center_box(m->geom);
        container_set_current_geom(con, &center_box);

        focus_container(con);
        lift_container(con);
        arrange();
    } else {
        if (container_get_hidden(con)) {
            show_container(con);
        } else {
            hide_container(con);
        }
    }
}
