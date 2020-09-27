# Julia wrapper for header: wlr_switch.h
# Automatically generated using Clang.jl


function wlr_switch_init(switch_device, impl)
    ccall((:wlr_switch_init, wlr_switch), Cvoid, (Ptr{wlr_switch}, Ptr{wlr_switch_impl}), switch_device, impl)
end

function wlr_switch_destroy(switch_device)
    ccall((:wlr_switch_destroy, wlr_switch), Cvoid, (Ptr{wlr_switch},), switch_device)
end
