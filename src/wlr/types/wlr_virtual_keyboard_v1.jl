# Julia wrapper for header: wlr_virtual_keyboard_v1.h
# Automatically generated using Clang.jl


function wlr_virtual_keyboard_manager_v1_create(display)
    ccall((:wlr_virtual_keyboard_manager_v1_create, wlr_virtual_keyboard_v1), Ptr{wlr_virtual_keyboard_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_input_device_get_virtual_keyboard(wlr_dev)
    ccall((:wlr_input_device_get_virtual_keyboard, wlr_virtual_keyboard_v1), Ptr{wlr_virtual_keyboard_v1}, (Ptr{wlr_input_device},), wlr_dev)
end
list
    events::ANONYMOUS2_events
end
