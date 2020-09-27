# Julia wrapper for header: xwayland.h
# Automatically generated using Clang.jl


function wlr_xwayland_server_create(display, options)
    ccall((:wlr_xwayland_server_create, xwayland), Ptr{wlr_xwayland_server}, (Ptr{wl_display}, Ptr{wlr_xwayland_server_options}), display, options)
end

function wlr_xwayland_server_destroy(server)
    ccall((:wlr_xwayland_server_destroy, xwayland), Cvoid, (Ptr{wlr_xwayland_server},), server)
end

function wlr_xwayland_create(wl_display, compositor, lazy)
    ccall((:wlr_xwayland_create, xwayland), Ptr{wlr_xwayland}, (Ptr{wl_display}, Ptr{wlr_compositor}, Bool), wl_display, compositor, lazy)
end

function wlr_xwayland_destroy(wlr_xwayland)
    ccall((:wlr_xwayland_destroy, xwayland), Cvoid, (Ptr{wlr_xwayland},), wlr_xwayland)
end

function wlr_xwayland_set_cursor(wlr_xwayland, pixels, stride, width, height, hotspot_x, hotspot_y)
    ccall((:wlr_xwayland_set_cursor, xwayland), Cvoid, (Ptr{wlr_xwayland}, Ptr{UInt8}, UInt32, UInt32, UInt32, Int32, Int32), wlr_xwayland, pixels, stride, width, height, hotspot_x, hotspot_y)
end

function wlr_xwayland_surface_activate(surface, activated)
    ccall((:wlr_xwayland_surface_activate, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Bool), surface, activated)
end

function wlr_xwayland_surface_configure(surface, x, y, width, height)
    ccall((:wlr_xwayland_surface_configure, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Int16, Int16, UInt16, UInt16), surface, x, y, width, height)
end

function wlr_xwayland_surface_close(surface)
    ccall((:wlr_xwayland_surface_close, xwayland), Cvoid, (Ptr{wlr_xwayland_surface},), surface)
end

function wlr_xwayland_surface_set_maximized(surface, maximized)
    ccall((:wlr_xwayland_surface_set_maximized, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Bool), surface, maximized)
end

function wlr_xwayland_surface_set_fullscreen(surface, fullscreen)
    ccall((:wlr_xwayland_surface_set_fullscreen, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Bool), surface, fullscreen)
end

function wlr_xwayland_set_seat(xwayland, seat)
    ccall((:wlr_xwayland_set_seat, xwayland), Cvoid, (Ptr{wlr_xwayland}, Ptr{wlr_seat}), xwayland, seat)
end

function wlr_surface_is_xwayland_surface(surface)
    ccall((:wlr_surface_is_xwayland_surface, xwayland), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_xwayland_surface_from_wlr_surface(surface)
    ccall((:wlr_xwayland_surface_from_wlr_surface, xwayland), Ptr{wlr_xwayland_surface}, (Ptr{wlr_surface},), surface)
end

function wlr_xwayland_surface_ping(surface)
    ccall((:wlr_xwayland_surface_ping, xwayland), Cvoid, (Ptr{wlr_xwayland_surface},), surface)
end

function wlr_xwayland_or_surface_wants_focus(surface)
    ccall((:wlr_xwayland_or_surface_wants_focus, xwayland), Bool, (Ptr{wlr_xwayland_surface},), surface)
end
_timeout::wl_signal
end

struct wlr_xwayland_surface
    window_id::xcb_window_t
    xwm::Ptr{wlr_xwm}
    surface_id::UInt32
    link::wl_list
    unpaired_link::wl_list
    surface::Ptr{wlr_surface}
    x::Int16
    y::Int16
    width::UInt16
    height::UInt16
    saved_width::UInt16
    saved_height::UInt16
    override_redirect::Bool
    mapped::Bool
    title::Cstring
    class::Cstring
    instance::Cstring
    role::Cstring
    pid::pid_t
    has_utf8_title::Bool
    children::wl_list
    parent::Ptr{wlr_xwayland_surface}
    parent_link::wl_list
    window_type::Ptr{xcb_atom_t}
    window_type_len::Csize_t
    protocols::Ptr{xcb_atom_t}
    protocols_len::Csize_t
    decorations::UInt32
    hints::Ptr{wlr_xwayland_surface_hints}
    hints_urgency::UInt32
    size_hints::Ptr{wlr_xwayland_surface_size_hints}
    pinging::Bool
    ping_timer::Ptr{wl_event_source}
    modal::Bool
    fullscreen::Bool
    maximized_vert::Bool
    maximized_horz::Bool
    has_alpha::Bool
    events::ANONYMOUS3_events
    surface_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_xwayland_surface_configure_event
    surface::Ptr{wlr_xwayland_surface}
    x::Int16
    y::Int16
    width::UInt16
    height::UInt16
    mask::UInt16
end

struct wlr_xwayland_move_event
    surface::Ptr{wlr_xwayland_surface}
end

struct wlr_xwayland_resize_event
    surface::Ptr{wlr_xwayland_surface}
    edges::UInt32
end
