# Julia wrapper for header: wlr_tablet_tool.h
# Automatically generated using Clang.jl

 WLR_TABLET_TOOL_TYPE_PEN = 1
    WLR_TABLET_TOOL_TYPE_ERASER = 2
    WLR_TABLET_TOOL_TYPE_BRUSH = 3
    WLR_TABLET_TOOL_TYPE_PENCIL = 4
    WLR_TABLET_TOOL_TYPE_AIRBRUSH = 5
    WLR_TABLET_TOOL_TYPE_MOUSE = 6
    WLR_TABLET_TOOL_TYPE_LENS = 7
    WLR_TABLET_TOOL_TYPE_TOTEM = 8
end


struct ANONYMOUS1_events
    destroy::wl_signal
end

struct wlr_tablet_tool
    type::wlr_tablet_tool_type
    hardware_serial::UInt64
    hardware_wacom::UInt64
    tilt::Bool
    pressure::Bool
    distance::Bool
    rotation::Bool
    slider::Bool
    wheel::Bool
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
end

const wlr_tablet_impl = Cvoid

struct ANONYMOUS2_events
    axis::wl_signal
    proximity::wl_signal
    tip::wl_signal
    button::wl_signal
end

struct wlr_tablet
    impl::Ptr{wlr_tablet_impl}
    events::ANONYMOUS2_events
    name::Cstring
    paths::wlr_list
    data::Ptr{Cvoid}
end

@cenum wlr_tablet_tool_axes::UInt32 begin
    WLR_TABLET_TOOL_AXIS_X = 1
    WLR_TABLET_TOOL_AXIS_Y = 2
    WLR_TABLET_TOOL_AXIS_DISTANCE = 4
    WLR_TABLET_TOOL_AXIS_PRESSURE = 8
    WLR_TABLET_TOOL_AXIS_TILT_X = 16
    WLR_TABLET_TOOL_AXIS_TILT_Y = 32
    WLR_TABLET_TOOL_AXIS_ROTATION = 64
    WLR_TABLET_TOOL_AXIS_SLIDER = 128
    WLR_TABLET_TOOL_AXIS_WHEEL = 256
end


struct wlr_event_tablet_tool_axis
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    updated_axes::UInt32
    x::Cdouble
    y::Cdouble
    dx::Cdouble
    dy::Cdouble
    pressure::Cdouble
    distance::Cdouble
    tilt_x::Cdouble
    tilt_y::Cdouble
    rotation::Cdouble
    slider::Cdouble
    wheel_delta::Cdouble
end

@cenum wlr_tablet_tool_proximity_state::UInt32 begin
    WLR_TABLET_TOOL_PROXIMITY_OUT = 0
    WLR_TABLET_TOOL_PROXIMITY_IN = 1
end


struct wlr_event_tablet_tool_proximity
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    x::Cdouble
    y::Cdouble
    state::wlr_tablet_tool_proximity_state
end

@cenum wlr_tablet_tool_tip_state::UInt32 begin
    WLR_TABLET_TOOL_TIP_UP = 0
    WLR_TABLET_TOOL_TIP_DOWN = 1
end


struct wlr_event_tablet_tool_tip
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    x::Cdouble
    y::Cdouble
    state::wlr_tablet_tool_tip_state
end

struct wlr_event_tablet_tool_button
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    button::UInt32
    state::wlr_button_state
end
