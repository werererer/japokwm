#include "scratchpad.h"

#include "monitor.h"
#include "server.h"
#include "tile/tileUtils.h"

// TODO rewrite this function so it is easier to read
void move_to_scratchpad(struct container *con, int position)
{
    if (!con)
        return;

    struct monitor *m = container_get_monitor(con);
    struct tagset *tagset = monitor_get_active_tagset(m);

    con->on_scratchpad = true;
    set_container_floating(con, fix_position, true);

    if (server.scratchpad->len== 0) {
        g_ptr_array_add(server.scratchpad, con);
    } else {
        int new_position = relative_index_to_absolute_index(0,
                position, server.scratchpad->len+1);
        g_ptr_array_insert(server.scratchpad, new_position, con);
    }

    remove_in_composed_list(tagset->list_set->container_lists, cmp_ptr, con);
    list_remove(tagset->list_set->focus_stack_normal, cmp_ptr, con);
    remove_in_composed_list(m->visual_stack_lists, cmp_ptr, con);

    container_damage_whole(con);
    focus_most_recent_container(tagset, FOCUS_NOOP);
    con->hidden = true;
    arrange();
}

void remove_container_from_scratchpad(struct container *con)
{
    list_remove(server.scratchpad, cmp_ptr, con);
    con->on_scratchpad = false;
}

void show_scratchpad()
{
    if (server.scratchpad->len <= 0)
        return;

    struct monitor *m = selected_monitor;
    struct tagset *tagset = monitor_get_active_tagset(m);
    struct container *sel = get_focused_container(m);
    struct container *con = g_ptr_array_index(server.scratchpad, 0);

    if (con->hidden) {
        g_ptr_array_add(tagset->list_set->floating_containers, con);
        g_ptr_array_insert(tagset->list_set->focus_stack_normal, 0, con);
        g_ptr_array_insert(m->floating_visual_stack, 0, con);

        resize(con, get_center_box(m->geom));

        con->hidden = false;
        focus_container(con, FOCUS_LIFT);
    } else {
        if (!sel->on_scratchpad) {
            focus_container(con, FOCUS_LIFT);
            return;
        }

        list_remove(tagset->list_set->floating_containers, cmp_ptr, con);
        list_remove(tagset->list_set->focus_stack_normal, cmp_ptr, con);
        list_remove(m->floating_visual_stack, cmp_ptr, con);

        list_remove(server.scratchpad, cmp_ptr, con);
        move_to_scratchpad(con, -1);
        con->hidden = true;
    }
    arrange();
}
