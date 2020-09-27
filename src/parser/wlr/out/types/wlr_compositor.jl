# Julia wrapper for header: wlr_compositor.h
# Automatically generated using Clang.jl


function wlr_compositor_create(display, renderer)
    ccall((:wlr_compositor_create, wlr_compositor), Ptr{wlr_compositor}, (Ptr{wl_display}, Ptr{wlr_renderer}), display, renderer)
end

function wlr_surface_is_subsurface(surface)
    ccall((:wlr_surface_is_subsurface, wlr_compositor), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_subsurface_from_wlr_surface(surface)
    ccall((:wlr_subsurface_from_wlr_surface, wlr_compositor), Ptr{wlr_subsurface}, (Ptr{wlr_surface},), surface)
end
