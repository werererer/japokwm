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

    con->tag_id = INVALID_TAG_ID;
    con->on_scratchpad = true;

    if (server.scratchpad->len== 0) {
        g_ptr_array_add(server.scratchpad, con);
    } else {
        int new_position = relative_index_to_absolute_index(0,
                position, server.scratchpad->len+1);
        g_ptr_array_insert(server.scratchpad, new_position, con);
    }

    struct tag *tag = monitor_get_active_tag(m);
    tagset_reload(tag);

    container_damage_whole(con);
    arrange();
    tag_focus_most_recent_container(tag);
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
        tag_this_focus_container(con);
        container_lift(con);
        return;
    }

    move_to_scratchpad(con, -1);
}

static void show_container(struct container *con)
{
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);

    container_set_hidden(con, false);
    container_set_tag_id(con, tag->id);
    container_set_floating(con, container_fix_position, true);
    struct wlr_box center_box = get_center_box(m->geom);
    container_set_current_geom(con, &center_box);

    tag_this_focus_container(con);
    container_lift(con);
    arrange();
}

void show_scratchpad()
{
    if (server.scratchpad->len <= 0)
        return;

    struct container *con = g_ptr_array_index(server.scratchpad, 0);
    struct monitor *m = server_get_selected_monitor();
    struct tag *tag = monitor_get_active_tag(m);
    bool visible_on_other_tag = !container_get_hidden(con) && tag->id != con->tag_id;
    if (visible_on_other_tag) {
        container_set_tag_id(con, tag->id);
        container_set_floating(con, container_fix_position, true);
        struct wlr_box center_box = get_center_box(m->geom);
        container_set_current_geom(con, &center_box);

        tag_this_focus_container(con);
        container_lift(con);
        arrange();
    } else {
        if (container_get_hidden(con)) {
            show_container(con);
        } else {
            hide_container(con);
        }
    }
}
