# Julia wrapper for header: wlr_tablet_tool.h
# Automatically generated using Clang.jl


function wlr_tablet_init(tablet, impl)
    ccall((:wlr_tablet_init, wlr_tablet_tool), Cvoid, (Ptr{wlr_tablet}, Ptr{wlr_tablet_impl}), tablet, impl)
end

function wlr_tablet_destroy(tablet)
    ccall((:wlr_tablet_destroy, wlr_tablet_tool), Cvoid, (Ptr{wlr_tablet},), tablet)
end
