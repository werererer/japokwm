# Julia wrapper for header: wlr_tablet_v2.h
# Automatically generated using Clang.jl


function wlr_tablet_create(manager, wlr_seat, wlr_device)
    ccall((:wlr_tablet_create, wlr_tablet_v2), Ptr{wlr_tablet_v2_tablet}, (Ptr{wlr_tablet_manager_v2}, Ptr{wlr_seat}, Ptr{wlr_input_device}), manager, wlr_seat, wlr_device)
end

function wlr_tablet_pad_create(manager, wlr_seat, wlr_device)
    ccall((:wlr_tablet_pad_create, wlr_tablet_v2), Ptr{wlr_tablet_v2_tablet_pad}, (Ptr{wlr_tablet_manager_v2}, Ptr{wlr_seat}, Ptr{wlr_input_device}), manager, wlr_seat, wlr_device)
end

function wlr_tablet_tool_create(manager, wlr_seat, wlr_tool)
    ccall((:wlr_tablet_tool_create, wlr_tablet_v2), Ptr{wlr_tablet_v2_tablet_tool}, (Ptr{wlr_tablet_manager_v2}, Ptr{wlr_seat}, Ptr{wlr_tablet_tool}), manager, wlr_seat, wlr_tool)
end

function wlr_tablet_v2_create(display)
    ccall((:wlr_tablet_v2_create, wlr_tablet_v2), Ptr{wlr_tablet_manager_v2}, (Ptr{wl_display},), display)
end

function wlr_send_tablet_v2_tablet_tool_proximity_in(tool, tablet, surface)
    ccall((:wlr_send_tablet_v2_tablet_tool_proximity_in, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Ptr{wlr_tablet_v2_tablet}, Ptr{wlr_surface}), tool, tablet, surface)
end

function wlr_send_tablet_v2_tablet_tool_down(tool)
    ccall((:wlr_send_tablet_v2_tablet_tool_down, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_send_tablet_v2_tablet_tool_up(tool)
    ccall((:wlr_send_tablet_v2_tablet_tool_up, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_send_tablet_v2_tablet_tool_motion(tool, x, y)
    ccall((:wlr_send_tablet_v2_tablet_tool_motion, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble, Cdouble), tool, x, y)
end

function wlr_send_tablet_v2_tablet_tool_pressure(tool, pressure)
    ccall((:wlr_send_tablet_v2_tablet_tool_pressure, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, pressure)
end

function wlr_send_tablet_v2_tablet_tool_distance(tool, distance)
    ccall((:wlr_send_tablet_v2_tablet_tool_distance, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, distance)
end

function wlr_send_tablet_v2_tablet_tool_tilt(tool, x, y)
    ccall((:wlr_send_tablet_v2_tablet_tool_tilt, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble, Cdouble), tool, x, y)
end

function wlr_send_tablet_v2_tablet_tool_rotation(tool, degrees)
    ccall((:wlr_send_tablet_v2_tablet_tool_rotation, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, degrees)
end

function wlr_send_tablet_v2_tablet_tool_slider(tool, position)
    ccall((:wlr_send_tablet_v2_tablet_tool_slider, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, position)
end

function wlr_send_tablet_v2_tablet_tool_wheel(tool, degrees, clicks)
    ccall((:wlr_send_tablet_v2_tablet_tool_wheel, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble, Int32), tool, degrees, clicks)
end

function wlr_send_tablet_v2_tablet_tool_proximity_out(tool)
    ccall((:wlr_send_tablet_v2_tablet_tool_proximity_out, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_send_tablet_v2_tablet_tool_button(tool, button, state)
    ccall((:wlr_send_tablet_v2_tablet_tool_button, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, UInt32, zwp_tablet_pad_v2_button_state), tool, button, state)
end

function wlr_tablet_v2_tablet_tool_notify_proximity_in(tool, tablet, surface)
    ccall((:wlr_tablet_v2_tablet_tool_notify_proximity_in, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Ptr{wlr_tablet_v2_tablet}, Ptr{wlr_surface}), tool, tablet, surface)
end

function wlr_tablet_v2_tablet_tool_notify_down(tool)
    ccall((:wlr_tablet_v2_tablet_tool_notify_down, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_tablet_v2_tablet_tool_notify_up(tool)
    ccall((:wlr_tablet_v2_tablet_tool_notify_up, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_tablet_v2_tablet_tool_notify_motion(tool, x, y)
    ccall((:wlr_tablet_v2_tablet_tool_notify_motion, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble, Cdouble), tool, x, y)
end

function wlr_tablet_v2_tablet_tool_notify_pressure(tool, pressure)
    ccall((:wlr_tablet_v2_tablet_tool_notify_pressure, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, pressure)
end

function wlr_tablet_v2_tablet_tool_notify_distance(tool, distance)
    ccall((:wlr_tablet_v2_tablet_tool_notify_distance, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, distance)
end

function wlr_tablet_v2_tablet_tool_notify_tilt(tool, x, y)
    ccall((:wlr_tablet_v2_tablet_tool_notify_tilt, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble, Cdouble), tool, x, y)
end

function wlr_tablet_v2_tablet_tool_notify_rotation(tool, degrees)
    ccall((:wlr_tablet_v2_tablet_tool_notify_rotation, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, degrees)
end

function wlr_tablet_v2_tablet_tool_notify_slider(tool, position)
    ccall((:wlr_tablet_v2_tablet_tool_notify_slider, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble), tool, position)
end

function wlr_tablet_v2_tablet_tool_notify_wheel(tool, degrees, clicks)
    ccall((:wlr_tablet_v2_tablet_tool_notify_wheel, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Cdouble, Int32), tool, degrees, clicks)
end

function wlr_tablet_v2_tablet_tool_notify_proximity_out(tool)
    ccall((:wlr_tablet_v2_tablet_tool_notify_proximity_out, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_tablet_v2_tablet_tool_notify_button(tool, button, state)
    ccall((:wlr_tablet_v2_tablet_tool_notify_button, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, UInt32, zwp_tablet_pad_v2_button_state), tool, button, state)
end

function wlr_tablet_tool_v2_start_grab(tool, grab)
    ccall((:wlr_tablet_tool_v2_start_grab, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool}, Ptr{wlr_tablet_tool_v2_grab}), tool, grab)
end

function wlr_tablet_tool_v2_end_grab(tool)
    ccall((:wlr_tablet_tool_v2_end_grab, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_tablet_tool_v2_start_implicit_grab(tool)
    ccall((:wlr_tablet_tool_v2_start_implicit_grab, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_tablet_tool_v2_has_implicit_grab(tool)
    ccall((:wlr_tablet_tool_v2_has_implicit_grab, wlr_tablet_v2), Bool, (Ptr{wlr_tablet_v2_tablet_tool},), tool)
end

function wlr_send_tablet_v2_tablet_pad_enter(pad, tablet, surface)
    ccall((:wlr_send_tablet_v2_tablet_pad_enter, wlr_tablet_v2), UInt32, (Ptr{wlr_tablet_v2_tablet_pad}, Ptr{wlr_tablet_v2_tablet}, Ptr{wlr_surface}), pad, tablet, surface)
end

function wlr_send_tablet_v2_tablet_pad_button(pad, button, time, state)
    ccall((:wlr_send_tablet_v2_tablet_pad_button, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, Csize_t, UInt32, zwp_tablet_pad_v2_button_state), pad, button, time, state)
end

function wlr_send_tablet_v2_tablet_pad_strip(pad, strip, position, finger, time)
    ccall((:wlr_send_tablet_v2_tablet_pad_strip, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, UInt32, Cdouble, Bool, UInt32), pad, strip, position, finger, time)
end

function wlr_send_tablet_v2_tablet_pad_ring(pad, ring, position, finger, time)
    ccall((:wlr_send_tablet_v2_tablet_pad_ring, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, UInt32, Cdouble, Bool, UInt32), pad, ring, position, finger, time)
end

function wlr_send_tablet_v2_tablet_pad_leave(pad, surface)
    ccall((:wlr_send_tablet_v2_tablet_pad_leave, wlr_tablet_v2), UInt32, (Ptr{wlr_tablet_v2_tablet_pad}, Ptr{wlr_surface}), pad, surface)
end

function wlr_send_tablet_v2_tablet_pad_mode(pad, group, mode, time)
    ccall((:wlr_send_tablet_v2_tablet_pad_mode, wlr_tablet_v2), UInt32, (Ptr{wlr_tablet_v2_tablet_pad}, Csize_t, UInt32, UInt32), pad, group, mode, time)
end

function wlr_tablet_v2_tablet_pad_notify_enter(pad, tablet, surface)
    ccall((:wlr_tablet_v2_tablet_pad_notify_enter, wlr_tablet_v2), UInt32, (Ptr{wlr_tablet_v2_tablet_pad}, Ptr{wlr_tablet_v2_tablet}, Ptr{wlr_surface}), pad, tablet, surface)
end

function wlr_tablet_v2_tablet_pad_notify_button(pad, button, time, state)
    ccall((:wlr_tablet_v2_tablet_pad_notify_button, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, Csize_t, UInt32, zwp_tablet_pad_v2_button_state), pad, button, time, state)
end

function wlr_tablet_v2_tablet_pad_notify_strip(pad, strip, position, finger, time)
    ccall((:wlr_tablet_v2_tablet_pad_notify_strip, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, UInt32, Cdouble, Bool, UInt32), pad, strip, position, finger, time)
end

function wlr_tablet_v2_tablet_pad_notify_ring(pad, ring, position, finger, time)
    ccall((:wlr_tablet_v2_tablet_pad_notify_ring, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, UInt32, Cdouble, Bool, UInt32), pad, ring, position, finger, time)
end

function wlr_tablet_v2_tablet_pad_notify_leave(pad, surface)
    ccall((:wlr_tablet_v2_tablet_pad_notify_leave, wlr_tablet_v2), UInt32, (Ptr{wlr_tablet_v2_tablet_pad}, Ptr{wlr_surface}), pad, surface)
end

function wlr_tablet_v2_tablet_pad_notify_mode(pad, group, mode, time)
    ccall((:wlr_tablet_v2_tablet_pad_notify_mode, wlr_tablet_v2), UInt32, (Ptr{wlr_tablet_v2_tablet_pad}, Csize_t, UInt32, UInt32), pad, group, mode, time)
end

function wlr_tablet_v2_end_grab(pad)
    ccall((:wlr_tablet_v2_end_grab, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad},), pad)
end

function wlr_tablet_v2_start_grab(pad, grab)
    ccall((:wlr_tablet_v2_start_grab, wlr_tablet_v2), Cvoid, (Ptr{wlr_tablet_v2_tablet_pad}, Ptr{wlr_tablet_pad_v2_grab}), pad, grab)
end

function wlr_surface_accepts_tablet_v2(tablet, surface)
    ccall((:wlr_surface_accepts_tablet_v2, wlr_tablet_v2), Bool, (Ptr{wlr_tablet_v2_tablet}, Ptr{wlr_surface}), tablet, surface)
end
