# Julia wrapper for header: wlr_pointer_gestures_v1.h
# Automatically generated using Clang.jl


function wlr_pointer_gestures_v1_create(display)
    ccall((:wlr_pointer_gestures_v1_create, wlr_pointer_gestures_v1), Ptr{wlr_pointer_gestures_v1}, (Ptr{wl_display},), display)
end

function wlr_pointer_gestures_v1_send_swipe_begin(gestures, seat, time_msec, fingers)
    ccall((:wlr_pointer_gestures_v1_send_swipe_begin, wlr_pointer_gestures_v1), Cvoid, (Ptr{wlr_pointer_gestures_v1}, Ptr{wlr_seat}, UInt32, UInt32), gestures, seat, time_msec, fingers)
end

function wlr_pointer_gestures_v1_send_swipe_update(gestures, seat, time_msec, dx, dy)
    ccall((:wlr_pointer_gestures_v1_send_swipe_update, wlr_pointer_gestures_v1), Cvoid, (Ptr{wlr_pointer_gestures_v1}, Ptr{wlr_seat}, UInt32, Cdouble, Cdouble), gestures, seat, time_msec, dx, dy)
end

function wlr_pointer_gestures_v1_send_swipe_end(gestures, seat, time_msec, cancelled)
    ccall((:wlr_pointer_gestures_v1_send_swipe_end, wlr_pointer_gestures_v1), Cvoid, (Ptr{wlr_pointer_gestures_v1}, Ptr{wlr_seat}, UInt32, Bool), gestures, seat, time_msec, cancelled)
end

function wlr_pointer_gestures_v1_send_pinch_begin(gestures, seat, time_msec, fingers)
    ccall((:wlr_pointer_gestures_v1_send_pinch_begin, wlr_pointer_gestures_v1), Cvoid, (Ptr{wlr_pointer_gestures_v1}, Ptr{wlr_seat}, UInt32, UInt32), gestures, seat, time_msec, fingers)
end

function wlr_pointer_gestures_v1_send_pinch_update(gestures, seat, time_msec, dx, dy, scale, rotation)
    ccall((:wlr_pointer_gestures_v1_send_pinch_update, wlr_pointer_gestures_v1), Cvoid, (Ptr{wlr_pointer_gestures_v1}, Ptr{wlr_seat}, UInt32, Cdouble, Cdouble, Cdouble, Cdouble), gestures, seat, time_msec, dx, dy, scale, rotation)
end

function wlr_pointer_gestures_v1_send_pinch_end(gestures, seat, time_msec, cancelled)
    ccall((:wlr_pointer_gestures_v1_send_pinch_end, wlr_pointer_gestures_v1), Cvoid, (Ptr{wlr_pointer_gestures_v1}, Ptr{wlr_seat}, UInt32, Bool), gestures, seat, time_msec, cancelled)
end
