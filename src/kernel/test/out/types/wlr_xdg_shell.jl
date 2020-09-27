# Julia wrapper for header: wlr_xdg_shell.h
# Automatically generated using Clang.jl


function wlr_xdg_shell_create(display)
    ccall((:wlr_xdg_shell_create, wlr_xdg_shell), Ptr{wlr_xdg_shell}, (Ptr{wl_display},), display)
end

function wlr_xdg_surface_from_resource(resource)
    ccall((:wlr_xdg_surface_from_resource, wlr_xdg_shell), Ptr{wlr_xdg_surface}, (Ptr{wl_resource},), resource)
end

function wlr_xdg_surface_from_popup_resource(resource)
    ccall((:wlr_xdg_surface_from_popup_resource, wlr_xdg_shell), Ptr{wlr_xdg_surface}, (Ptr{wl_resource},), resource)
end

function wlr_xdg_surface_from_toplevel_resource(resource)
    ccall((:wlr_xdg_surface_from_toplevel_resource, wlr_xdg_shell), Ptr{wlr_xdg_surface}, (Ptr{wl_resource},), resource)
end

function wlr_xdg_surface_ping(surface)
    ccall((:wlr_xdg_surface_ping, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_surface},), surface)
end

function wlr_xdg_toplevel_set_size(surface, width, height)
    ccall((:wlr_xdg_toplevel_set_size, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface}, UInt32, UInt32), surface, width, height)
end

function wlr_xdg_toplevel_set_activated(surface, activated)
    ccall((:wlr_xdg_toplevel_set_activated, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface}, Bool), surface, activated)
end

function wlr_xdg_toplevel_set_maximized(surface, maximized)
    ccall((:wlr_xdg_toplevel_set_maximized, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface}, Bool), surface, maximized)
end

function wlr_xdg_toplevel_set_fullscreen(surface, fullscreen)
    ccall((:wlr_xdg_toplevel_set_fullscreen, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface}, Bool), surface, fullscreen)
end

function wlr_xdg_toplevel_set_resizing(surface, resizing)
    ccall((:wlr_xdg_toplevel_set_resizing, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface}, Bool), surface, resizing)
end

function wlr_xdg_toplevel_set_tiled(surface, tiled_edges)
    ccall((:wlr_xdg_toplevel_set_tiled, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface}, UInt32), surface, tiled_edges)
end

function wlr_xdg_toplevel_send_close(surface)
    ccall((:wlr_xdg_toplevel_send_close, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_surface},), surface)
end

function wlr_xdg_popup_destroy(surface)
    ccall((:wlr_xdg_popup_destroy, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_surface},), surface)
end

function wlr_xdg_positioner_get_geometry(positioner)
    ccall((:wlr_xdg_positioner_get_geometry, wlr_xdg_shell), wlr_box, (Ptr{wlr_xdg_positioner},), positioner)
end

function wlr_xdg_popup_get_anchor_point(popup, toplevel_sx, toplevel_sy)
    ccall((:wlr_xdg_popup_get_anchor_point, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_popup}, Ptr{Cint}, Ptr{Cint}), popup, toplevel_sx, toplevel_sy)
end

function wlr_xdg_popup_get_toplevel_coords(popup, popup_sx, popup_sy, toplevel_sx, toplevel_sy)
    ccall((:wlr_xdg_popup_get_toplevel_coords, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_popup}, Cint, Cint, Ptr{Cint}, Ptr{Cint}), popup, popup_sx, popup_sy, toplevel_sx, toplevel_sy)
end

function wlr_xdg_popup_unconstrain_from_box(popup, toplevel_sx_box)
    ccall((:wlr_xdg_popup_unconstrain_from_box, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_popup}, Ptr{wlr_box}), popup, toplevel_sx_box)
end

function wlr_positioner_invert_x(positioner)
    ccall((:wlr_positioner_invert_x, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_positioner},), positioner)
end

function wlr_positioner_invert_y(positioner)
    ccall((:wlr_positioner_invert_y, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_positioner},), positioner)
end

function wlr_xdg_surface_surface_at(surface, sx, sy, sub_x, sub_y)
    ccall((:wlr_xdg_surface_surface_at, wlr_xdg_shell), Ptr{wlr_surface}, (Ptr{wlr_xdg_surface}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), surface, sx, sy, sub_x, sub_y)
end

function wlr_surface_is_xdg_surface(surface)
    ccall((:wlr_surface_is_xdg_surface, wlr_xdg_shell), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_xdg_surface_from_wlr_surface(surface)
    ccall((:wlr_xdg_surface_from_wlr_surface, wlr_xdg_shell), Ptr{wlr_xdg_surface}, (Ptr{wlr_surface},), surface)
end

function wlr_xdg_surface_get_geometry(surface, box)
    ccall((:wlr_xdg_surface_get_geometry, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_surface}, Ptr{wlr_box}), surface, box)
end

function wlr_xdg_surface_for_each_surface(surface, iterator, user_data)
    ccall((:wlr_xdg_surface_for_each_surface, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_surface}, wlr_surface_iterator_func_t, Ptr{Cvoid}), surface, iterator, user_data)
end

function wlr_xdg_surface_schedule_configure(surface)
    ccall((:wlr_xdg_surface_schedule_configure, wlr_xdg_shell), UInt32, (Ptr{wlr_xdg_surface},), surface)
end

function wlr_xdg_surface_for_each_popup(surface, iterator, user_data)
    ccall((:wlr_xdg_surface_for_each_popup, wlr_xdg_shell), Cvoid, (Ptr{wlr_xdg_surface}, wlr_surface_iterator_func_t, Ptr{Cvoid}), surface, iterator, user_data)
end
