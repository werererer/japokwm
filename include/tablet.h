#ifndef _JAPOKWM_INPUT_TABLET_H
#define _JAPOKWM_INPUT_TABLET_H
#include <wlr/types/wlr_layer_shell_v1.h>

struct seat;
struct wlr_tablet_tool;

struct tablet {
	struct wl_list link;
	struct seat_device *seat_device;
	struct wlr_tablet_v2_tablet *tablet_v2;
};

enum tablet_tool_mode {
	TABLET_TOOL_MODE_ABSOLUTE,
	TABLET_TOOL_MODE_RELATIVE,
};

struct tablet_tool {
	struct seat *seat;
	struct tablet *tablet;
	struct wlr_tablet_v2_tablet_tool *tablet_v2_tool;

	enum tablet_tool_mode mode;
	double tilt_x, tilt_y;

	struct wl_listener set_cursor;
	struct wl_listener tool_destroy;
};

struct tablet_pad {
	struct wl_list link;
	struct seat_device *seat_device;
	struct tablet *tablet;
	struct wlr_tablet_v2_tablet_pad *tablet_v2_pad;

	struct wl_listener attach;
	struct wl_listener button;
	struct wl_listener ring;
	struct wl_listener strip;

	struct wlr_surface *current_surface;
	struct wl_listener surface_destroy;

	struct wl_listener tablet_destroy;
};

struct tablet *tablet_create(struct seat *seat,
		struct seat_device *device);

void configure_tablet(struct tablet *tablet);

void tablet_destroy(struct tablet *tablet);

void tablet_tool_configure(struct tablet *tablet,
		struct wlr_tablet_tool *wlr_tool);

struct tablet_pad *tablet_pad_create(struct seat *seat,
		struct seat_device *device);

void configure_tablet_pad(struct tablet_pad *tablet_pad);

void tablet_pad_destroy(struct tablet_pad *tablet_pad);

void tablet_pad_notify_enter(struct tablet_pad *tablet_pad,
		struct wlr_surface *surface);

#endif
