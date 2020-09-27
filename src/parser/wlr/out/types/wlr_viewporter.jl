# Julia wrapper for header: wlr_viewporter.h
# Automatically generated using Clang.jl


function wlr_viewporter_create(display)
    ccall((:wlr_viewporter_create, wlr_viewporter), Ptr{wlr_viewporter}, (Ptr{wl_display},), display)
end
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    surface_destroy::wl_listener
    surface_commit::wl_listener
end
