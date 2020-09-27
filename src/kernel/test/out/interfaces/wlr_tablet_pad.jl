# Julia wrapper for header: wlr_tablet_pad.h
# Automatically generated using Clang.jl


function wlr_tablet_pad_init(pad, impl)
    ccall((:wlr_tablet_pad_init, wlr_tablet_pad), Cvoid, (Ptr{wlr_tablet_pad}, Ptr{wlr_tablet_pad_impl}), pad, impl)
end

function wlr_tablet_pad_destroy(pad)
    ccall((:wlr_tablet_pad_destroy, wlr_tablet_pad), Cvoid, (Ptr{wlr_tablet_pad},), pad)
end
