# Julia wrapper for header: wlr_layer_shell_v1.h
# Automatically generated using Clang.jl


function wlr_layer_shell_v1_create(display)
    ccall((:wlr_layer_shell_v1_create, wlr_layer_shell_v1), Ptr{wlr_layer_shell_v1}, (Ptr{wl_display},), display)
end

function wlr_layer_surface_v1_configure(surface, width, height)
    ccall((:wlr_layer_surface_v1_configure, wlr_layer_shell_v1), Cvoid, (Ptr{wlr_layer_surface_v1}, UInt32, UInt32), surface, width, height)
end

function wlr_layer_surface_v1_close(surface)
    ccall((:wlr_layer_surface_v1_close, wlr_layer_shell_v1), Cvoid, (Ptr{wlr_layer_surface_v1},), surface)
end

function wlr_surface_is_layer_surface(surface)
    ccall((:wlr_surface_is_layer_surface, wlr_layer_shell_v1), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_layer_surface_v1_from_wlr_surface(surface)
    ccall((:wlr_layer_surface_v1_from_wlr_surface, wlr_layer_shell_v1), Ptr{wlr_layer_surface_v1}, (Ptr{wlr_surface},), surface)
end

function wlr_layer_surface_v1_for_each_surface(surface, iterator, user_data)
    ccall((:wlr_layer_surface_v1_for_each_surface, wlr_layer_shell_v1), Cvoid, (Ptr{wlr_layer_surface_v1}, wlr_surface_iterator_func_t, Ptr{Cvoid}), surface, iterator, user_data)
end

function wlr_layer_surface_v1_surface_at(surface, sx, sy, sub_x, sub_y)
    ccall((:wlr_layer_surface_v1_surface_at, wlr_layer_shell_v1), Ptr{wlr_surface}, (Ptr{wlr_layer_surface_v1}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), surface, sx, sy, sub_x, sub_y)
end
