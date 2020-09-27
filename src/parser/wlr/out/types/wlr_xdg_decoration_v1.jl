# Julia wrapper for header: wlr_xdg_decoration_v1.h
# Automatically generated using Clang.jl


function wlr_xdg_decoration_manager_v1_create(display)
    ccall((:wlr_xdg_decoration_manager_v1_create, wlr_xdg_decoration_v1), Ptr{wlr_xdg_decoration_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_xdg_toplevel_decoration_v1_set_mode(decoration, mode)
    ccall((:wlr_xdg_toplevel_decoration_v1_set_mode, wlr_xdg_decoration_v1), UInt32, (Ptr{wlr_xdg_toplevel_decoration_v1}, wlr_xdg_toplevel_decoration_v1_mode), decoration, mode)
end
 wlr_xdg_toplevel_decoration_v1_configure
    link::wl_list
    surface_configure::Ptr{wlr_xdg_surface_configure}
    mode::wlr_xdg_toplevel_decoration_v1_mode
end

struct ANONYMOUS2_events
    destroy::wl_signal
    request_mode::wl_signal
end

struct wlr_xdg_toplevel_decoration_v1
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_xdg_surface}
    manager::Ptr{wlr_xdg_decoration_manager_v1}
    link::wl_list
    added::Bool
    current_mode::wlr_xdg_toplevel_decoration_v1_mode
    client_pending_mode::wlr_xdg_toplevel_decoration_v1_mode
    server_pending_mode::wlr_xdg_toplevel_decoration_v1_mode
    configure_list::wl_list
    events::ANONYMOUS2_events
    surface_destroy::wl_listener
    surface_configure::wl_listener
    surface_ack_configure::wl_listener
    surface_commit::wl_listener
    data::Ptr{Cvoid}
end
