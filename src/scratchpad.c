#include "scratchpad.h"

#include "monitor.h"
#include "tile/tileUtils.h"

struct wl_list scratchpad;

static struct container *get_container_on_scratchpad(int i);

static struct container *get_container_on_scratchpad(int i)
{
    if (wl_list_empty(&containers))
        return NULL;
    if (abs(i) > wl_list_length(&containers))
        return NULL;

    struct container *con;
    if (i >= 0) {
        struct wl_list *pos = &scratchpad;
        while (i >= 0) {
            if (pos->next)
                pos = pos->next;
            i--;
        }
        con = wl_container_of(pos, con, scratchpad_link);
    } else { // i < 0
        struct wl_list *pos = &scratchpad;
        while (i < 0) {
            pos = pos->prev;
            i++;
        }
        con = wl_container_of(pos, con, scratchpad_link);
    }
    return con;
}

void move_to_scratchpad(struct container *con, int position)
{
    if (!con)
        return;

    struct monitor *m = con->m;

    if (wl_list_empty(&scratchpad) 
            || abs(position) > wl_list_length(&containers)
            || position == 0) {
        wl_list_insert(&scratchpad, &con->scratchpad_link);
    } else {
        struct container *con1 = get_container_on_scratchpad(position);
        wl_list_insert(&con1->scratchpad_link, &con->scratchpad_link);
    }

    con->on_scratchpad = true;
    set_container_floating(con, true);
    wl_list_remove(&con->mlink);
    wl_list_remove(&con->flink);
    wl_list_remove(&con->slink);
    container_damage_whole(con);
    focus_most_recent_container(m->ws_ids[0], FOCUS_NOOP);
    con->hidden = true;
    arrange();
}

void remove_container_from_scratchpad(struct container *con)
{
    wl_list_remove(&con->scratchpad_link);
    con->on_scratchpad = false;
}

void show_scratchpad()
{
    struct monitor *m = selected_monitor;
    struct container *sel = get_focused_container(m);
    struct container *con = wl_container_of(scratchpad.next, con, scratchpad_link);

    if (con->hidden) {
        wl_list_insert(&containers, &con->mlink);
        wl_list_insert(&focus_stack, &con->flink);
        wl_list_insert(&stack, &con->slink);
        update_container_stack_positions(m);
        set_container_geom(con, get_center_box(m->geom));
        con->hidden = false;
        focus_container(con, FOCUS_LIFT);
    } else {
        if (!sel->on_scratchpad) {
            focus_container(con, FOCUS_LIFT);
            return;
        }

        wl_list_remove(&con->scratchpad_link);
        move_to_scratchpad(con, -1);
    }
    arrange();
}
