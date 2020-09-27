# Julia wrapper for header: wlr_pointer.h
# Automatically generated using Clang.jl

ONYMOUS1_events
    motion::wl_signal
    motion_absolute::wl_signal
    button::wl_signal
    axis::wl_signal
    frame::wl_signal
    swipe_begin::wl_signal
    swipe_update::wl_signal
    swipe_end::wl_signal
    pinch_begin::wl_signal
    pinch_update::wl_signal
    pinch_end::wl_signal
end

struct wlr_pointer
    impl::Ptr{wlr_pointer_impl}
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
end

struct wlr_event_pointer_motion
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    delta_x::Cdouble
    delta_y::Cdouble
    unaccel_dx::Cdouble
    unaccel_dy::Cdouble
end

struct wlr_event_pointer_motion_absolute
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    x::Cdouble
    y::Cdouble
end

struct wlr_event_pointer_button
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    button::UInt32
    state::wlr_button_state
end

@cenum wlr_axis_source::UInt32 begin
    WLR_AXIS_SOURCE_WHEEL = 0
    WLR_AXIS_SOURCE_FINGER = 1
    WLR_AXIS_SOURCE_CONTINUOUS = 2
    WLR_AXIS_SOURCE_WHEEL_TILT = 3
end

@cenum wlr_axis_orientation::UInt32 begin
    WLR_AXIS_ORIENTATION_VERTICAL = 0
    WLR_AXIS_ORIENTATION_HORIZONTAL = 1
end


struct wlr_event_pointer_axis
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    source::wlr_axis_source
    orientation::wlr_axis_orientation
    delta::Cdouble
    delta_discrete::Int32
end

struct wlr_event_pointer_swipe_begin
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
end

struct wlr_event_pointer_swipe_update
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
    dx::Cdouble
    dy::Cdouble
end

struct wlr_event_pointer_swipe_end
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    cancelled::Bool
end

struct wlr_event_pointer_pinch_begin
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
end

struct wlr_event_pointer_pinch_update
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
    dx::Cdouble
    dy::Cdouble
    scale::Cdouble
    rotation::Cdouble
end

struct wlr_event_pointer_pinch_end
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    cancelled::Bool
end
