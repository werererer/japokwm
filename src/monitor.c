#include "monitor.h"
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_list.h>
#include <stdlib.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_output_damage.h>
#include <assert.h>

#include "ipc-server.h"
#include "parseConfig.h"
#include "render/render.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "workspace.h"
#include "lib/actions/actions.h"

struct wl_list stack;
struct wl_list focus_stack;
struct wl_list containers;
struct wl_list layer_stack;
struct wl_list popups;
struct wl_list sticky_stack;

struct wl_list mons;
struct monitor *selected_monitor = NULL;

static void handle_output_damage_frame(struct wl_listener *listener, void *data);
static void handle_output_mode(struct wl_listener *listener, void *data);

void create_monitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *output = data;
    struct monitor *m;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(output, wlr_output_preferred_mode(output));

    /* Allocates and configures monitor state using configured rules */
    m = output->data = calloc(1, sizeof(struct monitor));

    m->wlr_output = output;
    m->root = create_root();

    /* damage tracking must be initialized before setting the workspace because
     * it to damage a region */
    m->damage = wlr_output_damage_create(m->wlr_output);
    m->damage_frame.notify = handle_output_damage_frame;
    wl_signal_add(&m->damage->events.frame, &m->damage_frame);

    set_next_unoccupied_workspace(m, get_workspace(0));

    // TODO fix this
    /* for (int i = 0; i < monrule_count; i++) { */
    /*     struct mon_rule r = monrules[i]; */
    /*     if (!r.name || strstr(output->name, r.name)) { */
    /*         m->mfact = r.mfact; */
    /*         wlr_output_set_scale(output, r.scale); */
    /*         wlr_xcursor_manager_load(server.cursorMgr, r.scale); */
    /*         printf("create monitor\n"); */
    /*         set_selected_layout(m->ws, r.lt); */
    /*         wlr_output_set_transform(output, WL_OUTPUT_TRANSFORM_NORMAL); */
    /*         break; */
    /*     } */
    /* } */
    /* Set up event listeners */
    m->destroy.notify = destroy_monitor;
    wl_signal_add(&output->events.destroy, &m->destroy);
    m->mode.notify = handle_output_mode;
    wl_signal_add(&output->events.mode, &m->mode);

    bool is_first_monitor = wl_list_empty(&mons);
    wl_list_insert(&mons, &m->link);
    if (is_first_monitor)
        set_selected_monitor(m);
    load_default_layout(L, m);

    /* Adds this to the output layout. The add_auto function arranges outputs
     * from left-to-right in the order they appear. A more sophisticated
     * compositor would let the user configure the arrangement of outputs in the
     * layout.
     *
     * The output layout utility automatically adds a wl_output global to the
     * display, which Wayland clients can see to find out information about the
     * output (such as DPI, scale factor, manufacturer, etc).
     */
    wlr_output_layout_add_auto(server.output_layout, output);

    wlr_output_enable(output, 1);

    if (!wlr_output_commit(output))
        return;
}

static void handle_output_damage_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);

    if (!m->wlr_output->enabled) {
        return;
    }

    /* Check if we can scan-out the primary view. */
    bool needs_frame;
    pixman_region32_t damage;
    pixman_region32_init(&damage);
    if (!wlr_output_damage_attach_render(m->damage, &needs_frame, &damage))
        goto damage_finish;

    if (!needs_frame) {
        wlr_output_rollback(m->wlr_output);
        pixman_region32_fini(&damage);
        return;
    }

    render_frame(m, &damage);

damage_finish:
    pixman_region32_fini(&damage);
}

static void handle_output_mode(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, mode);
    if (!m)
        return;
    arrange_monitor(m);
}

void focusmon(int i)
{
    selected_monitor = dirtomon(i);
    focus_top_container(selected_monitor, FOCUS_LIFT);
}

void destroy_monitor(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, destroy);
    printf("destroy_monitor\n");

    set_workspace(m, NULL);
    destroy_root(m->root);
    wl_list_remove(&m->link);
}

void center_mouse_in_monitor(struct monitor *m)
{
    if (!m)
        return;

    int xcenter = m->geom.x + m->geom.width/2;
    int ycenter = m->geom.y + m->geom.height/2;
    wlr_cursor_warp(server.cursor, NULL, xcenter, ycenter);
}

void set_selected_monitor(struct monitor *m)
{
    assert(m);
    if (selected_monitor == m)
        return;

    selected_monitor = m;
    set_workspace(m, m->ws);
    int x = server.cursor->x;
    int y = server.cursor->y;

    focus_container(xytocontainer(x, y), m, FOCUS_NOOP);
}

void push_selected_workspace(struct monitor *m, struct workspace *ws)
{
    if (!m || !ws)
        return;
    set_workspace(m, ws);
}

struct monitor *dirtomon(int dir)
{
    struct monitor *m;

    if (dir > 0) {
        if (selected_monitor->link.next == &mons)
            return wl_container_of(mons.next, m, link);
        return wl_container_of(selected_monitor->link.next, m, link);
    } else {
        if (selected_monitor->link.prev == &mons)
            return wl_container_of(mons.prev, m, link);
        return wl_container_of(selected_monitor->link.prev, m, link);
    }
}

struct monitor *outputtomon(struct wlr_output *output)
{
    struct monitor *m;
    bool found = false;
    wl_list_for_each(m, &mons, link) {
        if (m->wlr_output == output) {
            found = true;
            break;
        }
    }
    return found ? m : NULL;
}

struct monitor *xytomon(double x, double y)
{
    struct wlr_output *o = wlr_output_layout_output_at(server.output_layout, x, y);
    return o ? o->data : NULL;
}

void load_layout(lua_State *L, struct monitor *m, const char *layout_name) {
    m->ws->layout.name = layout_name;

    char *config_path = get_config_file("layouts");
    char file[NUM_CHARS];
    strcpy(file, "");
    join_path(file, config_path);
    join_path(file, layout_name);
    join_path(file, "enter.lua");

    if (!file_exists(file))
        return;

    if (luaL_loadfile(L, file)) {
        lua_pop(L, 1);
        return;
    }
    lua_pcall(L, 0, 0, 0);
    lua_pop(L, 1);
}

void load_default_layout(lua_State *L, struct monitor *m)
{
    load_layout(L, m, default_layout.name);
}
