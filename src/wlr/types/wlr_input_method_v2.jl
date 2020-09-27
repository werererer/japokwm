# Julia wrapper for header: wlr_input_method_v2.h
# Automatically generated using Clang.jl


function wlr_input_method_manager_v2_create(display)
    ccall((:wlr_input_method_manager_v2_create, wlr_input_method_v2), Ptr{wlr_input_method_manager_v2}, (Ptr{wl_display},), display)
end

function wlr_input_method_v2_send_activate(input_method)
    ccall((:wlr_input_method_v2_send_activate, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2},), input_method)
end

function wlr_input_method_v2_send_deactivate(input_method)
    ccall((:wlr_input_method_v2_send_deactivate, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2},), input_method)
end

function wlr_input_method_v2_send_surrounding_text(input_method, text, cursor, anchor)
    ccall((:wlr_input_method_v2_send_surrounding_text, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2}, Cstring, UInt32, UInt32), input_method, text, cursor, anchor)
end

function wlr_input_method_v2_send_content_type(input_method, hint, purpose)
    ccall((:wlr_input_method_v2_send_content_type, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2}, UInt32, UInt32), input_method, hint, purpose)
end

function wlr_input_method_v2_send_text_change_cause(input_method, cause)
    ccall((:wlr_input_method_v2_send_text_change_cause, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2}, UInt32), input_method, cause)
end

function wlr_input_method_v2_send_done(input_method)
    ccall((:wlr_input_method_v2_send_done, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2},), input_method)
end

function wlr_input_method_v2_send_unavailable(input_method)
    ccall((:wlr_input_method_v2_send_unavailable, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_v2},), input_method)
end

function wlr_input_method_keyboard_grab_v2_send_key(keyboard_grab, time, key, state)
    ccall((:wlr_input_method_keyboard_grab_v2_send_key, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_keyboard_grab_v2}, UInt32, UInt32, UInt32), keyboard_grab, time, key, state)
end

function wlr_input_method_keyboard_grab_v2_send_modifiers(keyboard_grab, modifiers)
    ccall((:wlr_input_method_keyboard_grab_v2_send_modifiers, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_keyboard_grab_v2}, Ptr{wlr_keyboard_modifiers}), keyboard_grab, modifiers)
end

function wlr_input_method_keyboard_grab_v2_set_keyboard(keyboard_grab, keyboard)
    ccall((:wlr_input_method_keyboard_grab_v2_set_keyboard, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_keyboard_grab_v2}, Ptr{wlr_keyboard}), keyboard_grab, keyboard)
end

function wlr_input_method_keyboard_grab_v2_destroy(keyboard_grab)
    ccall((:wlr_input_method_keyboard_grab_v2_destroy, wlr_input_method_v2), Cvoid, (Ptr{wlr_input_method_keyboard_grab_v2},), keyboard_grab)
end
