# Julia wrapper for header: wlr_keyboard_shortcuts_inhibit_v1.h
# Automatically generated using Clang.jl


function wlr_keyboard_shortcuts_inhibit_v1_create(display)
    ccall((:wlr_keyboard_shortcuts_inhibit_v1_create, wlr_keyboard_shortcuts_inhibit_v1), Ptr{wlr_keyboard_shortcuts_inhibit_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_keyboard_shortcuts_inhibitor_v1_activate(inhibitor)
    ccall((:wlr_keyboard_shortcuts_inhibitor_v1_activate, wlr_keyboard_shortcuts_inhibit_v1), Cvoid, (Ptr{wlr_keyboard_shortcuts_inhibitor_v1},), inhibitor)
end

function wlr_keyboard_shortcuts_inhibitor_v1_deactivate(inhibitor)
    ccall((:wlr_keyboard_shortcuts_inhibitor_v1_deactivate, wlr_keyboard_shortcuts_inhibit_v1), Cvoid, (Ptr{wlr_keyboard_shortcuts_inhibitor_v1},), inhibitor)
end
