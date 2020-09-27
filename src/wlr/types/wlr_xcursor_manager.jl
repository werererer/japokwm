# Julia wrapper for header: wlr_xcursor_manager.h
# Automatically generated using Clang.jl


function wlr_xcursor_manager_create(name, size)
    ccall((:wlr_xcursor_manager_create, wlr_xcursor_manager), Ptr{wlr_xcursor_manager}, (Cstring, UInt32), name, size)
end

function wlr_xcursor_manager_destroy(manager)
    ccall((:wlr_xcursor_manager_destroy, wlr_xcursor_manager), Cvoid, (Ptr{wlr_xcursor_manager},), manager)
end

function wlr_xcursor_manager_load(manager, scale)
    ccall((:wlr_xcursor_manager_load, wlr_xcursor_manager), Bool, (Ptr{wlr_xcursor_manager}, Cfloat), manager, scale)
end

function wlr_xcursor_manager_get_xcursor(manager, name, scale)
    ccall((:wlr_xcursor_manager_get_xcursor, wlr_xcursor_manager), Ptr{wlr_xcursor}, (Ptr{wlr_xcursor_manager}, Cstring, Cfloat), manager, name, scale)
end

function wlr_xcursor_manager_set_cursor_image(manager, name, cursor)
    ccall((:wlr_xcursor_manager_set_cursor_image, wlr_xcursor_manager), Cvoid, (Ptr{wlr_xcursor_manager}, Cstring, Ptr{wlr_cursor}), manager, name, cursor)
end
