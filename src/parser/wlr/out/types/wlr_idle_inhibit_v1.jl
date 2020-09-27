# Julia wrapper for header: wlr_idle_inhibit_v1.h
# Automatically generated using Clang.jl


function wlr_idle_inhibit_v1_create(display)
    ccall((:wlr_idle_inhibit_v1_create, wlr_idle_inhibit_v1), Ptr{wlr_idle_inhibit_manager_v1}, (Ptr{wl_display},), display)
end
_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS2_events
    destroy::wl_signal
end

struct wlr_idle_inhibitor_v1
    surface::Ptr{wlr_surface}
    resource::Ptr{wl_resource}
    surface_destroy::wl_listener
    link::wl_list
    events::ANONYMOUS2_events
    data::Ptr{Cvoid}
end
