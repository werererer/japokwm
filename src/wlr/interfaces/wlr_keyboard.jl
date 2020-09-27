# Julia wrapper for header: wlr_keyboard.h
# Automatically generated using Clang.jl


function wlr_keyboard_init(keyboard, impl)
    ccall((:wlr_keyboard_init, wlr_keyboard), Cvoid, (Ptr{wlr_keyboard}, Ptr{wlr_keyboard_impl}), keyboard, impl)
end

function wlr_keyboard_destroy(keyboard)
    ccall((:wlr_keyboard_destroy, wlr_keyboard), Cvoid, (Ptr{wlr_keyboard},), keyboard)
end

function wlr_keyboard_notify_key(keyboard, event)
    ccall((:wlr_keyboard_notify_key, wlr_keyboard), Cvoid, (Ptr{wlr_keyboard}, Ptr{wlr_event_keyboard_key}), keyboard, event)
end

function wlr_keyboard_notify_modifiers(keyboard, mods_depressed, mods_latched, mods_locked, group)
    ccall((:wlr_keyboard_notify_modifiers, wlr_keyboard), Cvoid, (Ptr{wlr_keyboard}, UInt32, UInt32, UInt32, UInt32), keyboard, mods_depressed, mods_latched, mods_locked, group)
end
