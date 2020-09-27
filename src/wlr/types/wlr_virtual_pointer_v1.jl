# Julia wrapper for header: wlr_virtual_pointer_v1.h
# Automatically generated using Clang.jl


function wlr_virtual_pointer_manager_v1_create(display)
    ccall((:wlr_virtual_pointer_manager_v1_create, wlr_virtual_pointer_v1), Ptr{wlr_virtual_pointer_manager_v1}, (Ptr{wl_display},), display)
end
uct ANONYMOUS2_events
    destroy::wl_signal
end

struct wlr_virtual_pointer_v1
    input_device::wlr_input_device
    resource::Ptr{wl_resource}
    axis_event::NTuple{2, wlr_event_pointer_axis}
    axis::wl_pointer_axis
    axis_valid::NTuple{2, Bool}
    link::wl_list
    events::ANONYMOUS2_events
end

struct wlr_virtual_pointer_v1_new_pointer_event
    new_pointer::Ptr{wlr_virtual_pointer_v1}
    suggested_seat::Ptr{wlr_seat}
    suggested_output::Ptr{wlr_output}
end
