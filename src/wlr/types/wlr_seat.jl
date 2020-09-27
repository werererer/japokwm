# Julia wrapper for header: wlr_seat.h
# Automatically generated using Clang.jl


function wlr_seat_create(display, name)
    ccall((:wlr_seat_create, wlr_seat), Ptr{wlr_seat}, (Ptr{wl_display}, Cstring), display, name)
end

function wlr_seat_destroy(wlr_seat)
    ccall((:wlr_seat_destroy, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_client_for_wl_client(wlr_seat, wl_client)
    ccall((:wlr_seat_client_for_wl_client, wlr_seat), Ptr{wlr_seat_client}, (Ptr{wlr_seat}, Ptr{wl_client}), wlr_seat, wl_client)
end

function wlr_seat_set_capabilities(wlr_seat, capabilities)
    ccall((:wlr_seat_set_capabilities, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32), wlr_seat, capabilities)
end

function wlr_seat_set_name(wlr_seat, name)
    ccall((:wlr_seat_set_name, wlr_seat), Cvoid, (Ptr{wlr_seat}, Cstring), wlr_seat, name)
end

function wlr_seat_pointer_surface_has_focus(wlr_seat, surface)
    ccall((:wlr_seat_pointer_surface_has_focus, wlr_seat), Bool, (Ptr{wlr_seat}, Ptr{wlr_surface}), wlr_seat, surface)
end

function wlr_seat_pointer_enter(wlr_seat, surface, sx, sy)
    ccall((:wlr_seat_pointer_enter, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_surface}, Cdouble, Cdouble), wlr_seat, surface, sx, sy)
end

function wlr_seat_pointer_clear_focus(wlr_seat)
    ccall((:wlr_seat_pointer_clear_focus, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_pointer_send_motion(wlr_seat, time_msec, sx, sy)
    ccall((:wlr_seat_pointer_send_motion, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Cdouble, Cdouble), wlr_seat, time_msec, sx, sy)
end

function wlr_seat_pointer_send_button(wlr_seat, time_msec, button, state)
    ccall((:wlr_seat_pointer_send_button, wlr_seat), UInt32, (Ptr{wlr_seat}, UInt32, UInt32, wlr_button_state), wlr_seat, time_msec, button, state)
end

function wlr_seat_pointer_send_axis(wlr_seat, time_msec, orientation, value, value_discrete, source)
    ccall((:wlr_seat_pointer_send_axis, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, wlr_axis_orientation, Cdouble, Int32, wlr_axis_source), wlr_seat, time_msec, orientation, value, value_discrete, source)
end

function wlr_seat_pointer_send_frame(wlr_seat)
    ccall((:wlr_seat_pointer_send_frame, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_pointer_notify_enter(wlr_seat, surface, sx, sy)
    ccall((:wlr_seat_pointer_notify_enter, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_surface}, Cdouble, Cdouble), wlr_seat, surface, sx, sy)
end

function wlr_seat_pointer_notify_clear_focus(wlr_seat)
    ccall((:wlr_seat_pointer_notify_clear_focus, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_pointer_warp(wlr_seat, sx, sy)
    ccall((:wlr_seat_pointer_warp, wlr_seat), Cvoid, (Ptr{wlr_seat}, Cdouble, Cdouble), wlr_seat, sx, sy)
end

function wlr_seat_pointer_notify_motion(wlr_seat, time_msec, sx, sy)
    ccall((:wlr_seat_pointer_notify_motion, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Cdouble, Cdouble), wlr_seat, time_msec, sx, sy)
end

function wlr_seat_pointer_notify_button(wlr_seat, time_msec, button, state)
    ccall((:wlr_seat_pointer_notify_button, wlr_seat), UInt32, (Ptr{wlr_seat}, UInt32, UInt32, wlr_button_state), wlr_seat, time_msec, button, state)
end

function wlr_seat_pointer_notify_axis(wlr_seat, time_msec, orientation, value, value_discrete, source)
    ccall((:wlr_seat_pointer_notify_axis, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, wlr_axis_orientation, Cdouble, Int32, wlr_axis_source), wlr_seat, time_msec, orientation, value, value_discrete, source)
end

function wlr_seat_pointer_notify_frame(wlr_seat)
    ccall((:wlr_seat_pointer_notify_frame, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_pointer_start_grab(wlr_seat, grab)
    ccall((:wlr_seat_pointer_start_grab, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_seat_pointer_grab}), wlr_seat, grab)
end

function wlr_seat_pointer_end_grab(wlr_seat)
    ccall((:wlr_seat_pointer_end_grab, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_pointer_has_grab(seat)
    ccall((:wlr_seat_pointer_has_grab, wlr_seat), Bool, (Ptr{wlr_seat},), seat)
end

function wlr_seat_set_keyboard(seat, dev)
    ccall((:wlr_seat_set_keyboard, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_input_device}), seat, dev)
end

function wlr_seat_get_keyboard(seat)
    ccall((:wlr_seat_get_keyboard, wlr_seat), Ptr{wlr_keyboard}, (Ptr{wlr_seat},), seat)
end

function wlr_seat_keyboard_send_key(seat, time_msec, key, state)
    ccall((:wlr_seat_keyboard_send_key, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, UInt32, UInt32), seat, time_msec, key, state)
end

function wlr_seat_keyboard_send_modifiers(seat, modifiers)
    ccall((:wlr_seat_keyboard_send_modifiers, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_keyboard_modifiers}), seat, modifiers)
end

function wlr_seat_keyboard_enter(seat, surface, keycodes, num_keycodes, modifiers)
    ccall((:wlr_seat_keyboard_enter, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_surface}, Ptr{UInt32}, Csize_t, Ptr{wlr_keyboard_modifiers}), seat, surface, keycodes, num_keycodes, modifiers)
end

function wlr_seat_keyboard_clear_focus(wlr_seat)
    ccall((:wlr_seat_keyboard_clear_focus, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_keyboard_notify_key(seat, time_msec, key, state)
    ccall((:wlr_seat_keyboard_notify_key, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, UInt32, UInt32), seat, time_msec, key, state)
end

function wlr_seat_keyboard_notify_modifiers(seat, modifiers)
    ccall((:wlr_seat_keyboard_notify_modifiers, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_keyboard_modifiers}), seat, modifiers)
end

function wlr_seat_keyboard_notify_enter(seat, surface, keycodes, num_keycodes, modifiers)
    ccall((:wlr_seat_keyboard_notify_enter, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_surface}, Ptr{UInt32}, Csize_t, Ptr{wlr_keyboard_modifiers}), seat, surface, keycodes, num_keycodes, modifiers)
end

function wlr_seat_keyboard_notify_clear_focus(wlr_seat)
    ccall((:wlr_seat_keyboard_notify_clear_focus, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_keyboard_start_grab(wlr_seat, grab)
    ccall((:wlr_seat_keyboard_start_grab, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_seat_keyboard_grab}), wlr_seat, grab)
end

function wlr_seat_keyboard_end_grab(wlr_seat)
    ccall((:wlr_seat_keyboard_end_grab, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_keyboard_has_grab(seat)
    ccall((:wlr_seat_keyboard_has_grab, wlr_seat), Bool, (Ptr{wlr_seat},), seat)
end

function wlr_seat_touch_get_point(seat, touch_id)
    ccall((:wlr_seat_touch_get_point, wlr_seat), Ptr{wlr_touch_point}, (Ptr{wlr_seat}, Int32), seat, touch_id)
end

function wlr_seat_touch_point_focus(seat, surface, time_msec, touch_id, sx, sy)
    ccall((:wlr_seat_touch_point_focus, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_surface}, UInt32, Int32, Cdouble, Cdouble), seat, surface, time_msec, touch_id, sx, sy)
end

function wlr_seat_touch_point_clear_focus(seat, time_msec, touch_id)
    ccall((:wlr_seat_touch_point_clear_focus, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Int32), seat, time_msec, touch_id)
end

function wlr_seat_touch_send_down(seat, surface, time_msec, touch_id, sx, sy)
    ccall((:wlr_seat_touch_send_down, wlr_seat), UInt32, (Ptr{wlr_seat}, Ptr{wlr_surface}, UInt32, Int32, Cdouble, Cdouble), seat, surface, time_msec, touch_id, sx, sy)
end

function wlr_seat_touch_send_up(seat, time_msec, touch_id)
    ccall((:wlr_seat_touch_send_up, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Int32), seat, time_msec, touch_id)
end

function wlr_seat_touch_send_motion(seat, time_msec, touch_id, sx, sy)
    ccall((:wlr_seat_touch_send_motion, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Int32, Cdouble, Cdouble), seat, time_msec, touch_id, sx, sy)
end

function wlr_seat_touch_notify_down(seat, surface, time_msec, touch_id, sx, sy)
    ccall((:wlr_seat_touch_notify_down, wlr_seat), UInt32, (Ptr{wlr_seat}, Ptr{wlr_surface}, UInt32, Int32, Cdouble, Cdouble), seat, surface, time_msec, touch_id, sx, sy)
end

function wlr_seat_touch_notify_up(seat, time_msec, touch_id)
    ccall((:wlr_seat_touch_notify_up, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Int32), seat, time_msec, touch_id)
end

function wlr_seat_touch_notify_motion(seat, time_msec, touch_id, sx, sy)
    ccall((:wlr_seat_touch_notify_motion, wlr_seat), Cvoid, (Ptr{wlr_seat}, UInt32, Int32, Cdouble, Cdouble), seat, time_msec, touch_id, sx, sy)
end

function wlr_seat_touch_num_points(seat)
    ccall((:wlr_seat_touch_num_points, wlr_seat), Cint, (Ptr{wlr_seat},), seat)
end

function wlr_seat_touch_start_grab(wlr_seat, grab)
    ccall((:wlr_seat_touch_start_grab, wlr_seat), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_seat_touch_grab}), wlr_seat, grab)
end

function wlr_seat_touch_end_grab(wlr_seat)
    ccall((:wlr_seat_touch_end_grab, wlr_seat), Cvoid, (Ptr{wlr_seat},), wlr_seat)
end

function wlr_seat_touch_has_grab(seat)
    ccall((:wlr_seat_touch_has_grab, wlr_seat), Bool, (Ptr{wlr_seat},), seat)
end

function wlr_seat_validate_grab_serial(seat, serial)
    ccall((:wlr_seat_validate_grab_serial, wlr_seat), Bool, (Ptr{wlr_seat}, UInt32), seat, serial)
end

function wlr_seat_validate_pointer_grab_serial(seat, origin, serial)
    ccall((:wlr_seat_validate_pointer_grab_serial, wlr_seat), Bool, (Ptr{wlr_seat}, Ptr{wlr_surface}, UInt32), seat, origin, serial)
end

function wlr_seat_validate_touch_grab_serial(seat, origin, serial, point_ptr)
    ccall((:wlr_seat_validate_touch_grab_serial, wlr_seat), Bool, (Ptr{wlr_seat}, Ptr{wlr_surface}, UInt32, Ptr{Ptr{wlr_touch_point}}), seat, origin, serial, point_ptr)
end

function wlr_seat_client_next_serial(client)
    ccall((:wlr_seat_client_next_serial, wlr_seat), UInt32, (Ptr{wlr_seat_client},), client)
end

function wlr_seat_client_validate_event_serial(client, serial)
    ccall((:wlr_seat_client_validate_event_serial, wlr_seat), Bool, (Ptr{wlr_seat_client}, UInt32), client, serial)
end

function wlr_seat_client_from_resource(resource)
    ccall((:wlr_seat_client_from_resource, wlr_seat), Ptr{wlr_seat_client}, (Ptr{wl_resource},), resource)
end

function wlr_seat_client_from_pointer_resource(resource)
    ccall((:wlr_seat_client_from_pointer_resource, wlr_seat), Ptr{wlr_seat_client}, (Ptr{wl_resource},), resource)
end

function wlr_surface_accepts_touch(wlr_seat, surface)
    ccall((:wlr_surface_accepts_touch, wlr_seat), Bool, (Ptr{wlr_seat}, Ptr{wlr_surface}), wlr_seat, surface)
end
