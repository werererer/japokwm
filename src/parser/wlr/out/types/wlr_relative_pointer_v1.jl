# Julia wrapper for header: wlr_relative_pointer_v1.h
# Automatically generated using Clang.jl


function wlr_relative_pointer_manager_v1_create(display)
    ccall((:wlr_relative_pointer_manager_v1_create, wlr_relative_pointer_v1), Ptr{wlr_relative_pointer_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_relative_pointer_manager_v1_send_relative_motion(manager, seat, time_usec, dx, dy, dx_unaccel, dy_unaccel)
    ccall((:wlr_relative_pointer_manager_v1_send_relative_motion, wlr_relative_pointer_v1), Cvoid, (Ptr{wlr_relative_pointer_manager_v1}, Ptr{wlr_seat}, UInt64, Cdouble, Cdouble, Cdouble, Cdouble), manager, seat, time_usec, dx, dy, dx_unaccel, dy_unaccel)
end

function wlr_relative_pointer_v1_from_resource(resource)
    ccall((:wlr_relative_pointer_v1_from_resource, wlr_relative_pointer_v1), Ptr{wlr_relative_pointer_v1}, (Ptr{wl_resource},), resource)
end
