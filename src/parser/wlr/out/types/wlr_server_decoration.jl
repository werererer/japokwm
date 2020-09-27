# Julia wrapper for header: wlr_server_decoration.h
# Automatically generated using Clang.jl


function wlr_server_decoration_manager_create(display)
    ccall((:wlr_server_decoration_manager_create, wlr_server_decoration), Ptr{wlr_server_decoration_manager}, (Ptr{wl_display},), display)
end

function wlr_server_decoration_manager_set_default_mode(manager, default_mode)
    ccall((:wlr_server_decoration_manager_set_default_mode, wlr_server_decoration), Cvoid, (Ptr{wlr_server_decoration_manager}, UInt32), manager, default_mode)
end
    data::Ptr{Cvoid}
end

struct ANONYMOUS2_events
    destroy::wl_signal
    mode::wl_signal
end

struct wlr_server_decoration
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    link::wl_list
    mode::UInt32
    events::ANONYMOUS2_events
    surface_destroy_listener::wl_listener
    data::Ptr{Cvoid}
end
