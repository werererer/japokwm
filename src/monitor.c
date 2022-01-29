#include "monitor.h"
#include <assert.h>
#include <stdlib.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>
#include <wlr/backend/wayland.h>
#include <wlr/backend/headless.h>
#include <wlr/backend/x11.h>

#include "ipc-server.h"
#include "render/render.h"
#include "server.h"
#include "tile/tileUtils.h"
#include "tag.h"
#include "utils/parseConfigUtils.h"
#include "layer_shell.h"
#include "rules/mon_rule.h"
#include "tagset.h"
#include "root.h"
#include "list_sets/container_stack_set.h"
#include "client.h"
#include "container.h"

static void handle_output_damage_frame(struct wl_listener *listener, void *data);
static void handle_output_frame(struct wl_listener *listener, void *data);
static void handle_output_mode(struct wl_listener *listener, void *data);
static void monitor_get_initial_tag(struct monitor *m, GList *tags);
static void prepare_output(struct wlr_output_configuration_head_v1 *config_head, struct wlr_output *wlr_output);
static void check_succeed(struct wlr_output_configuration_v1 *config, bool output_ok, bool test);
static bool output_test(struct wlr_output *wlr_output, bool output_ok, bool test);

void create_monitor(struct wl_listener *listener, void *data)
{
    /* This event is raised by the backend when a new output (aka a display or
     * monitor) becomes available. */
    struct wlr_output *output = data;

    /* The mode is a tuple of (width, height, refresh rate), and each
     * monitor supports only a specific set of modes. We just pick the
     * monitor's preferred mode; a more sophisticated compositor would let
     * the user configure it. */
    wlr_output_set_mode(output, wlr_output_preferred_mode(output));

    wlr_output_init_render(output, server.allocator, server.renderer);

    /* Allocates and configures monitor state using configured rules */
    struct monitor *m = output->data = calloc(1, sizeof(*m));

    m->tag_id = INVALID_TAG_ID;
    m->wlr_output = output;

    /* damage tracking must be initialized before setting the tag because
     * it to damage a region */
    m->damage = wlr_output_damage_create(m->wlr_output);
    m->damage_frame.notify = handle_output_damage_frame;
    wl_signal_add(&m->damage->events.frame, &m->damage_frame);

    m->frame.notify = handle_output_frame;
    wl_signal_add(&m->wlr_output->events.frame, &m->frame);

    /* Set up event listeners */
    m->destroy.notify = handle_destroy_monitor;
    wl_signal_add(&output->events.destroy, &m->destroy);
    m->mode.notify = handle_output_mode;
    wl_signal_add(&output->events.mode, &m->mode);

    bool is_first_monitor = server.mons->len == 0;
    g_ptr_array_add(server.mons, m);

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

    wlr_output_enable(output, true);

    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    m->root = create_root(m, m->geom);

    if (is_first_monitor) {
        server_set_selected_monitor(m);
        init_utils(L);
    }
    monitor_set_selected_tag(m, get_tag(0));
    assert(m->tag_id != INVALID_TAG_ID);

    if (is_first_monitor) {
        load_config(L);
        load_tags(server_get_tags(), server.default_layout->options->tag_names);
        server_allow_reloading_config();

        bitset_set(server.previous_bitset, server.previous_tag);

        call_on_start_function(server.event_handler);

        // reset all layouts
        for (GList *iterator = server_get_tags(); iterator; iterator = iterator->next) {
            struct tag *tag = iterator->data;
            // assert(tag->loaded_layouts->len == 0);
            tag->current_layout = server.default_layout->name;
            tag->previous_layout = server.default_layout->name;
        }
    }
    struct tag *tag = monitor_get_active_tag(m);
    tagset_focus_tag(tag);

    apply_mon_rules(server.default_layout->options->mon_rules, m);

    arrange();
    struct layout *lt = tag_get_layout(tag);
    set_root_color(m->root, lt->options->root_color);

    if (!wlr_output_commit(m->wlr_output))
        return;
}

void create_output(struct wlr_backend *backend, void *data)
{
    bool *done = data;
    if (*done) {
        return;
    }

    if (wlr_backend_is_wl(backend)) {
        wlr_wl_output_create(backend);
        *done = true;
    } else if (wlr_backend_is_headless(backend)) {
        wlr_headless_add_output(backend, 1920, 1080);
        *done = true;
    }
/* #if WLR_HAS_X11_BACKEND */
    else if (wlr_backend_is_x11(backend)) {
        wlr_x11_output_create(backend);
        *done = true;
    }
/* #endif */
}

int i = 0;
static void handle_output_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);
    // debug_print("frame: %i\n", i++);

    /* NOOP */
}

int j = 0;
static void handle_output_damage_frame(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, damage_frame);

    if (!m->wlr_output->enabled) {
        return;
    }

    /* Check if we can scan-out the primary view. */
    pixman_region32_t frame_damage;
    pixman_region32_init(&frame_damage);
    bool needs_frame;
    if (!wlr_output_damage_attach_render(m->damage, &needs_frame, &frame_damage)) {
        goto damage_finish;
    }

    if (!needs_frame) {
        wlr_output_rollback(m->wlr_output);
        goto damage_finish;
    }

    render_monitor(m, &frame_damage);

damage_finish:
    pixman_region32_fini(&frame_damage);
}

static void handle_output_mode(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, mode);
    m->geom = *wlr_output_layout_get_box(server.output_layout, m->wlr_output);
    arrange_monitor(m);
    arrange_layers(m);
}

static void monitor_get_initial_tag(struct monitor *m, GList *tags)
{
    struct tag *tag = find_next_unoccupied_tag(tags, get_tag(0));

    assert(tag != NULL);

    monitor_set_selected_tag(m, tag);
    int tag_id = tag->id;
    BitSet *bitset = bitset_create();
    bitset_set(bitset, tag_id);

    monitor_focus_tags(m, tag, bitset);
}

void handle_destroy_monitor(struct wl_listener *listener, void *data)
{
    struct monitor *m = wl_container_of(listener, m, destroy);

    wl_list_remove(&m->mode.link);
    wl_list_remove(&m->frame.link);
    wl_list_remove(&m->destroy.link);

    struct tag *tag = monitor_get_active_tag(m);
    for (GList *iterator = server_get_tags(); iterator; iterator = iterator->next) {
        struct tag *tag = iterator->data;
        if (tag->m == m) {
            tag_set_selected_monitor(tag, NULL);
        }
        if (tag->current_m == m) {
            tag_set_current_monitor(tag, NULL);
        }
    }

    GPtrArray *stack_list = tag_get_complete_stack_copy(tag);
    for (int i = 0; i < stack_list->len; i++) {
        struct container *con = g_ptr_array_index(stack_list, i);

        if (con->client->type == LAYER_SHELL && con->client->m == m) {
            // WARNING: the following code is dangerously bad
            // HACK:
            // FIXME: this needs to be done so that instead of segfaulting when
            // layer shell based programs are open it leaks the memory instead
            // this seems to be a bug in wlroots or wayland
            wl_list_remove(&con->client->commit.link);
            wl_list_remove(&con->client->destroy.link);
            con->tag_id = INVALID_TAG_ID;
        }
        if (con->client->m == m) {
            con->client->m = NULL;
        }
    }
    g_ptr_array_unref(stack_list);

    for (GList *iterator = server_get_tags(); iterator; iterator = iterator->next) {
        struct tag *tag = iterator->data;
        if (tag->current_m == m) {
            debug_print("unset ws: %i\n", tag->id);
            tag_set_current_monitor(tag, NULL);
        }
    }

    destroy_root(m->root);
    g_ptr_array_remove(server.mons, m);
    m->wlr_output->data = NULL;

    free(m);

    if (server.mons->len <= 0) {
        /* wl_display_terminate(server.wl_display); */
        return;
    }

    struct monitor *new_focused_monitor = g_ptr_array_index(server.mons, 0);
    server_set_selected_monitor(new_focused_monitor);
}

static void monitor_remove_link_to_tag(struct monitor *m, struct tag *tag)
{
    struct tag *old_tag = get_tag(m->tag_id);
    tag_set_selected_monitor(old_tag, NULL);
    m->tag_id = INVALID_TAG_ID;
}

void monitor_remove_link_to_own_tag(struct monitor *m)
{
    monitor_remove_link_to_tag(m, get_tag(m->tag_id));
}

static void monitor_create_link_to_tag(struct monitor *m, struct tag *tag)
{
    m->tag_id = tag->id;
    tag_set_selected_monitor(tag, m);
}

static void monitor_reparent(struct monitor *m, struct tag *tag)
{
    monitor_remove_link_to_own_tag(m);
    monitor_create_link_to_tag(m, tag);
}

static void monitor_swap_tags(struct monitor *m1, struct monitor *m2)
{
    struct tag *tag1 = get_tag(m1->tag_id);
    struct tag *tag2 = get_tag(m2->tag_id);

    monitor_create_link_to_tag(m1, tag2);
    monitor_create_link_to_tag(m2, tag1);
}

static bool tag_has_a_link_to_any_monitor(struct tag *tag)
{
    return tag->m != NULL;
}

static bool monitor_is_corrupted(struct monitor *m)
{
    struct tag *tag = get_tag(m->tag_id);

    // if we have no tag the monitor is not connected to any tag. therefore no
    // connection back exist and the connection can't be corrupted.
    if (!tag)
        return false;

    return m != tag->m;
}

static bool monitor_has_link_to_tag(struct monitor *m)
{
    return m->tag_id != INVALID_TAG_ID;
}

static bool monitor_should_swap_tag(struct monitor *m, struct tag *tag)
{
    if (!tag_has_a_link_to_any_monitor(tag))
        return false;

    if (!monitor_has_link_to_tag(m))
        return false;

    return true;
}

static bool monitor_should_use_the_next_empty_tag(struct monitor *m, struct tag *tag)
{
    if (!tag_has_a_link_to_any_monitor(tag))
        return false;
    if (monitor_has_link_to_tag(m))
        return false;
    return true;
}

static void monitor_use_the_next_empty_tag(struct monitor *m, struct tag *tag)
{
    struct tag *new_tag = find_next_unoccupied_tag(server_get_tags(), tag);
    monitor_create_link_to_tag(m, new_tag);
}

void monitor_set_selected_tag(struct monitor *m, struct tag *tag)
{
    assert(!monitor_is_corrupted(m));

    if (monitor_should_swap_tag(m, tag)) {
        monitor_swap_tags(m, tag->m);
    } else if (monitor_should_use_the_next_empty_tag(m, tag)) {
        monitor_use_the_next_empty_tag(m, tag);
    } else {
        monitor_reparent(m, tag);
    }
}

BitSet *monitor_get_tags(struct monitor *m)
{
    struct tag *sel_tag = monitor_get_active_tag(m);
    return sel_tag->tags;
}

void center_cursor_in_monitor(struct cursor *cursor, struct monitor *m)
{
    if (!m)
        return;

    int xcenter = m->geom.x + m->geom.width/2;
    int ycenter = m->geom.y + m->geom.height/2;
    wlr_cursor_warp(cursor->wlr_cursor, NULL, xcenter, ycenter);
}

void scale_monitor(struct monitor *m, float scale)
{
    struct wlr_output *output = m->wlr_output;
    m->scale = scale;
    wlr_output_set_scale(output, scale);
}

void transform_monitor(struct monitor *m, enum wl_output_transform transform)
{
    struct wlr_output *output = m->wlr_output;
    wlr_output_set_transform(output, transform);
}

void update_monitor_geometries()
{
    struct wlr_output_configuration_v1 *config = wlr_output_configuration_v1_create();
    
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        struct wlr_output_configuration_head_v1 *config_head = wlr_output_configuration_head_v1_create(config, m->wlr_output);

        arrange_layers(m);
        arrange_monitor(m);
        config_head->state.enabled = m->wlr_output->enabled;
        struct wlr_box *monitor_box = &m->geom;
        if (monitor_box) {
            config_head->state.x = monitor_box->x;
            config_head->state.y = monitor_box->y;
	}
    }
    wlr_output_manager_v1_set_configuration(server.output_mgr, config);
}

void focus_monitor(struct monitor *m)
{
    if (!m)
        return;
    if (server_get_selected_monitor() == m)
        return;

    struct tag *tag = monitor_get_active_tag(m);
    tagset_focus_tags(tag, tag->tags);
    server_set_selected_monitor(m);
}

struct monitor *output_to_monitor(struct wlr_output *output)
{
    for (int i = 0; i < server.mons->len; i++) {
        struct monitor *m = g_ptr_array_index(server.mons, i);
        if (m->wlr_output == output) {
            return m;
        }
    }

    return NULL;
}

struct monitor *xy_to_monitor(double x, double y)
{
    struct wlr_output *o = wlr_output_layout_output_at(server.output_layout, x, y);
    return o ? o->data : NULL;
}

inline struct tag *monitor_get_active_tag(struct monitor *m)
{
    if (!m)
        return NULL;

    struct tag *tag = get_tag(m->tag_id);
    // if this is not correct the whole application is in an undefined state
    if (tag && tag->m) {
        assert(tag->m == m);
    }
    return tag;
}

inline struct layout *get_layout_in_monitor(struct monitor *m)
{
    if (!m)
        return NULL;
    struct tag *tag = monitor_get_active_tag(m);
    struct layout *lt = tag_get_layout(tag);
    return lt;
}

struct root *monitor_get_active_root(struct monitor *m)
{
    struct root *root = m->root;
    return root;
}

struct wlr_box monitor_get_active_geom(struct monitor *m)
{
    struct tag *tag = monitor_get_active_tag(m);
    struct wlr_box geom = tag_get_active_geom(tag);
    return geom;
}

void handle_output_mgr_apply(struct wl_listener *listener, void *data)
{
	struct wlr_output_configuration_v1 *config = data;
	handle_output_mgr_apply_test(config, false);
}

// apply_output_config
void handle_output_mgr_apply_test(
        struct wlr_output_configuration_v1 *config, bool test)
{
    struct wlr_output_configuration_head_v1 *config_head;
    bool output_ok = true;
    wl_list_for_each(config_head, &config->heads, link) {
        struct wlr_output *wlr_output = config_head->state.output;

        wlr_output_enable(wlr_output, config_head->state.enabled);
       prepare_output(config_head, wlr_output);
       output_ok = output_test(wlr_output, output_ok, test);
    }
    check_succeed(config, output_ok, test);
    wlr_output_configuration_v1_destroy(config);
}

void handle_output_mgr_test(struct wl_listener *listener, void *data)
{
	struct wlr_output_configuration_v1 *config = data;
  handle_output_mgr_apply_test(config, true);
}

static void prepare_output(
        struct wlr_output_configuration_head_v1 *config_head,
        struct wlr_output *wlr_output)
{
    if (config_head->state.enabled) {
        if (config_head->state.mode)
            wlr_output_set_mode(wlr_output, config_head->state.mode);
        else
            wlr_output_set_custom_mode(wlr_output,
                    config_head->state.custom_mode.width,
                    config_head->state.custom_mode.height,
                    config_head->state.custom_mode.refresh);

        wlr_output_layout_move(server.output_layout, wlr_output,
                config_head->state.x, config_head->state.y);
        wlr_output_set_transform(wlr_output, config_head->state.transform);
        wlr_output_set_scale(wlr_output, config_head->state.scale);
    }
}

static bool output_test(struct wlr_output *wlr_output, bool output_ok,
        bool test)
{
    if (test) {
        output_ok &= wlr_output_test(wlr_output);
                    wlr_output_rollback(wlr_output);
    } else{
                    output_ok &= wlr_output_commit(wlr_output);
    }
    return output_ok;
}    
static void check_succeed(struct wlr_output_configuration_v1 *config,
        bool output_ok, bool test)
{
    if (output_ok) {
        wlr_output_configuration_v1_send_succeeded(config);
        if (!test)
	    update_monitor_geometries();
    }else{
        wlr_output_configuration_v1_send_failed(config);
    }
}
