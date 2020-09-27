# Julia wrapper for header: wlr_switch.h
# Automatically generated using Clang.jl

ONYMOUS1_events
    toggle::wl_signal
end

struct wlr_switch
    impl::Ptr{wlr_switch_impl}
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
end

@cenum wlr_switch_type::UInt32 begin
    WLR_SWITCH_TYPE_LID = 1
    WLR_SWITCH_TYPE_TABLET_MODE = 2
end

@cenum wlr_switch_state::UInt32 begin
    WLR_SWITCH_STATE_OFF = 0
    WLR_SWITCH_STATE_ON = 1
    WLR_SWITCH_STATE_TOGGLE = 2
end


struct wlr_event_switch_toggle
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    switch_type::wlr_switch_type
    switch_state::wlr_switch_state
end
