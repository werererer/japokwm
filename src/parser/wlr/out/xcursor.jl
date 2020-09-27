# Julia wrapper for header: xcursor.h
# Automatically generated using Clang.jl


function wlr_xcursor_theme_load(name, size)
    ccall((:wlr_xcursor_theme_load, xcursor), Ptr{wlr_xcursor_theme}, (Cstring, Cint), name, size)
end

function wlr_xcursor_theme_destroy(theme)
    ccall((:wlr_xcursor_theme_destroy, xcursor), Cvoid, (Ptr{wlr_xcursor_theme},), theme)
end

function wlr_xcursor_theme_get_cursor(theme, name)
    ccall((:wlr_xcursor_theme_get_cursor, xcursor), Ptr{wlr_xcursor}, (Ptr{wlr_xcursor_theme}, Cstring), theme, name)
end

function wlr_xcursor_frame(cursor, time)
    ccall((:wlr_xcursor_frame, xcursor), Cint, (Ptr{wlr_xcursor}, UInt32), cursor, time)
end

function wlr_xcursor_get_resize_name(edges)
    ccall((:wlr_xcursor_get_resize_name, xcursor), Cstring, (wlr_edges,), edges)
end
