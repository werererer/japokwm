# Julia wrapper for header: wlr_input_device.h
# Automatically generated using Clang.jl

BUTTON_RELEASED = 0
    WLR_BUTTON_PRESSED = 1
end

@cenum wlr_input_device_type::UInt32 begin
    WLR_INPUT_DEVICE_KEYBOARD = 0
    WLR_INPUT_DEVICE_POINTER = 1
    WLR_INPUT_DEVICE_TOUCH = 2
    WLR_INPUT_DEVICE_TABLET_TOOL = 3
    WLR_INPUT_DEVICE_TABLET_PAD = 4
    WLR_INPUT_DEVICE_SWITCH = 5
end


const wlr_input_device_impl = Cvoid

struct ANONYMOUS1_events
    destroy::wl_signal
end

struct wlr_input_device
    impl::Ptr{wlr_input_device_impl}
    type::wlr_input_device_type
    vendor::UInt32
    product::UInt32
    name::Cstring
    width_mm::Cdouble
    height_mm::Cdouble
    output_name::Cstring
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
    link::wl_list
end
