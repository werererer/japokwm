# Julia wrapper for header: wlr_keyboard_group.h
# Automatically generated using Clang.jl


function wlr_keyboard_group_create()
    ccall((:wlr_keyboard_group_create, wlr_keyboard_group), Ptr{wlr_keyboard_group}, ())
end

function wlr_keyboard_group_from_wlr_keyboard(keyboard)
    ccall((:wlr_keyboard_group_from_wlr_keyboard, wlr_keyboard_group), Ptr{wlr_keyboard_group}, (Ptr{wlr_keyboard},), keyboard)
end

function wlr_keyboard_group_add_keyboard(group, keyboard)
    ccall((:wlr_keyboard_group_add_keyboard, wlr_keyboard_group), Bool, (Ptr{wlr_keyboard_group}, Ptr{wlr_keyboard}), group, keyboard)
end

function wlr_keyboard_group_remove_keyboard(group, keyboard)
    ccall((:wlr_keyboard_group_remove_keyboard, wlr_keyboard_group), Cvoid, (Ptr{wlr_keyboard_group}, Ptr{wlr_keyboard}), group, keyboard)
end

function wlr_keyboard_group_destroy(group)
    ccall((:wlr_keyboard_group_destroy, wlr_keyboard_group), Cvoid, (Ptr{wlr_keyboard_group},), group)
end
