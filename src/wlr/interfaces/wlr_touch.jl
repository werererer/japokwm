# Julia wrapper for header: wlr_touch.h
# Automatically generated using Clang.jl


function wlr_touch_init(touch, impl)
    ccall((:wlr_touch_init, wlr_touch), Cvoid, (Ptr{wlr_touch}, Ptr{wlr_touch_impl}), touch, impl)
end

function wlr_touch_destroy(touch)
    ccall((:wlr_touch_destroy, wlr_touch), Cvoid, (Ptr{wlr_touch},), touch)
end
