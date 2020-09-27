# Julia wrapper for header: wlr_tablet_pad.h
# Automatically generated using Clang.jl

ONYMOUS1_events
    button::wl_signal
    ring::wl_signal
    strip::wl_signal
    attach_tablet::wl_signal
end

struct wlr_tablet_pad
    impl::Ptr{wlr_tablet_pad_impl}
    events::ANONYMOUS1_events
    button_count::Csize_t
    ring_count::Csize_t
    strip_count::Csize_t
    groups::wl_list
    paths::wlr_list
    data::Ptr{Cvoid}
end

struct wlr_tablet_pad_group
    link::wl_list
    button_count::Csize_t
    buttons::Ptr{UInt32}
    strip_count::Csize_t
    strips::Ptr{UInt32}
    ring_count::Csize_t
    rings::Ptr{UInt32}
    mode_count::UInt32
end

struct wlr_event_tablet_pad_button
    time_msec::UInt32
    button::UInt32
    state::wlr_button_state
    mode::UInt32
    group::UInt32
end

@cenum wlr_tablet_pad_ring_source::UInt32 begin
    WLR_TABLET_PAD_RING_SOURCE_UNKNOWN = 0
    WLR_TABLET_PAD_RING_SOURCE_FINGER = 1
end


struct wlr_event_tablet_pad_ring
    time_msec::UInt32
    source::wlr_tablet_pad_ring_source
    ring::UInt32
    position::Cdouble
    mode::UInt32
end

@cenum wlr_tablet_pad_strip_source::UInt32 begin
    WLR_TABLET_PAD_STRIP_SOURCE_UNKNOWN = 0
    WLR_TABLET_PAD_STRIP_SOURCE_FINGER = 1
end


struct wlr_event_tablet_pad_strip
    time_msec::UInt32
    source::wlr_tablet_pad_strip_source
    strip::UInt32
    position::Cdouble
    mode::UInt32
end
