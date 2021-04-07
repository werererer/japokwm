#include "scratchpad.h"

#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"

// TODO rewrite this function so it is easier to read
void move_to_scratchpad(struct container *con, int position)
{
    if (!con)
        return;

    struct monitor *m = con->m;
    struct workspace *ws = get_workspace_in_monitor(m);

    con->on_scratchpad = true;
    set_container_floating(con, fix_position, true);

    if (server.scratchpad.length == 0) {
        wlr_list_push(&server.scratchpad, con);
    } else {
        int new_position = relative_index_to_absolute_index(0,
                position, server.scratchpad.length+1);
        wlr_list_insert(&server.scratchpad, new_position, con);
    }

    wlr_list_remove_in_composed_list(&ws->container_lists, cmp_ptr, con);
    wlr_list_remove(&ws->focus_stack_normal, cmp_ptr, con);
    wlr_list_remove(&server.tiled_visual_stack, cmp_ptr, con);

    container_damage_whole(con);
    focus_most_recent_container(m->ws_ids[0], FOCUS_NOOP);
    con->hidden = true;
    arrange();
}

void remove_container_from_scratchpad(struct container *con)
{
    wlr_list_remove(&server.scratchpad, cmp_ptr, con);
    con->on_scratchpad = false;
}

void show_scratchpad()
{
    struct monitor *m = selected_monitor;
    struct workspace *ws = get_workspace_in_monitor(m);
    struct container *sel = get_focused_container(m);
    struct container *con = server.scratchpad.items[0];

    if (con->hidden) {
        wlr_list_push(&ws->floating_containers, con);
        wlr_list_insert(&ws->focus_stack_normal, 0, con);
        wlr_list_insert(&server.floating_visual_stack, 0, con);

        resize(con, get_center_box(m->geom));

        con->hidden = false;
        focus_container(con, FOCUS_LIFT);
    } else {
        if (!sel->on_scratchpad) {
            focus_container(con, FOCUS_LIFT);
            return;
        }

        wlr_list_remove(&ws->floating_containers, cmp_ptr, con);
        wlr_list_remove(&ws->focus_stack_normal, cmp_ptr, con);
        wlr_list_remove(&server.floating_visual_stack, cmp_ptr, con);

        wlr_list_remove(&server.scratchpad, cmp_ptr, con);
        move_to_scratchpad(con, -1);
        con->hidden = true;
    }
    arrange();
}
