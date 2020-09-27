# Julia wrapper for header: wlr_touch.h
# Automatically generated using Clang.jl

ONYMOUS1_events
    down::wl_signal
    up::wl_signal
    motion::wl_signal
    cancel::wl_signal
end

struct wlr_touch
    impl::Ptr{wlr_touch_impl}
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
end

struct wlr_event_touch_down
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
    x::Cdouble
    y::Cdouble
end

struct wlr_event_touch_up
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
end

struct wlr_event_touch_motion
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
    x::Cdouble
    y::Cdouble
end

struct wlr_event_touch_cancel
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
end
