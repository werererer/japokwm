#include "scratchpad.h"

#include "client.h"
#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "workspace.h"

// TODO rewrite this function so it is easier to read
void move_to_scratchpad(struct container *con, int position)
{
    if (!con)
        return;
    if (con->on_scratchpad) {
        remove_container_from_scratchpad(con);
    }

    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);
    // TODO: try to remove this
    container_set_floating(con, container_fix_position, false);

    con->client->ws_id = INVALID_WORKSPACE_ID;
    con->on_scratchpad = true;

    if (server.scratchpad->len== 0) {
        g_ptr_array_add(server.scratchpad, con);
    } else {
        int new_position = relative_index_to_absolute_index(0,
                position, server.scratchpad->len+1);
        g_ptr_array_insert(server.scratchpad, new_position, con);
    }

    /* remove_in_composed_list(tagset->list_set->container_lists, cmp_ptr, con); */
    /* list_remove(tagset->list_set->focus_stack_normal, cmp_ptr, con); */
    /* remove_in_composed_list(server.visual_stack_lists, cmp_ptr, con); */
    tagset_reload(tagset);

    container_damage_whole(con);
    struct workspace *ws = get_workspace(tagset->selected_ws_id);
    focus_most_recent_container(ws);
    con->hidden = true;
    arrange();
}

void remove_container_from_scratchpad(struct container *con)
{
    g_ptr_array_remove(server.scratchpad, con);
    con->on_scratchpad = false;
}

static void hide_container(struct container *con)
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);

    if (!sel->on_scratchpad) {
        focus_container(con);
        lift_container(con);
        return;
    }

    move_to_scratchpad(con, -1);
}

static void show_container(struct container *con)
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = monitor_get_active_workspace(m);

    con->hidden = false;
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
    struct monitor *m = selected_monitor;
    struct workspace *ws = monitor_get_active_workspace(m);
    bool visible_on_other_workspace = !con->hidden && ws->id != con->client->ws_id;
    if (visible_on_other_workspace) {
        container_set_workspace_id(con, ws->id);
        container_set_floating(con, container_fix_position, true);
        struct wlr_box center_box = get_center_box(m->geom);
        container_set_current_geom(con, &center_box);

        focus_container(con);
        lift_container(con);
        arrange();
    } else {
        if (con->hidden) {
            show_container(con);
        } else {
            hide_container(con);
        }
    }
}
