# Julia wrapper for header: wlr_idle.h
# Automatically generated using Clang.jl


function wlr_idle_create(display)
    ccall((:wlr_idle_create, wlr_idle), Ptr{wlr_idle}, (Ptr{wl_display},), display)
end

function wlr_idle_notify_activity(idle, seat)
    ccall((:wlr_idle_notify_activity, wlr_idle), Cvoid, (Ptr{wlr_idle}, Ptr{wlr_seat}), idle, seat)
end

function wlr_idle_set_enabled(idle, seat, enabled)
    ccall((:wlr_idle_set_enabled, wlr_idle), Cvoid, (Ptr{wlr_idle}, Ptr{wlr_seat}, Bool), idle, seat, enabled)
end

function wlr_idle_timeout_create(idle, seat, timeout)
    ccall((:wlr_idle_timeout_create, wlr_idle), Ptr{wlr_idle_timeout}, (Ptr{wlr_idle}, Ptr{wlr_seat}, UInt32), idle, seat, timeout)
end

function wlr_idle_timeout_destroy(timeout)
    ccall((:wlr_idle_timeout_destroy, wlr_idle), Cvoid, (Ptr{wlr_idle_timeout},), timeout)
end
