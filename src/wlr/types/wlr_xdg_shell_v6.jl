# Julia wrapper for header: wlr_xdg_shell_v6.h
# Automatically generated using Clang.jl


function wlr_xdg_shell_v6_create(display)
    ccall((:wlr_xdg_shell_v6_create, wlr_xdg_shell_v6), Ptr{wlr_xdg_shell_v6}, (Ptr{wl_display},), display)
end

function wlr_xdg_surface_v6_ping(surface)
    ccall((:wlr_xdg_surface_v6_ping, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_surface_v6},), surface)
end

function wlr_xdg_toplevel_v6_set_size(surface, width, height)
    ccall((:wlr_xdg_toplevel_v6_set_size, wlr_xdg_shell_v6), UInt32, (Ptr{wlr_xdg_surface_v6}, UInt32, UInt32), surface, width, height)
end

function wlr_xdg_toplevel_v6_set_activated(surface, activated)
    ccall((:wlr_xdg_toplevel_v6_set_activated, wlr_xdg_shell_v6), UInt32, (Ptr{wlr_xdg_surface_v6}, Bool), surface, activated)
end

function wlr_xdg_toplevel_v6_set_maximized(surface, maximized)
    ccall((:wlr_xdg_toplevel_v6_set_maximized, wlr_xdg_shell_v6), UInt32, (Ptr{wlr_xdg_surface_v6}, Bool), surface, maximized)
end

function wlr_xdg_toplevel_v6_set_fullscreen(surface, fullscreen)
    ccall((:wlr_xdg_toplevel_v6_set_fullscreen, wlr_xdg_shell_v6), UInt32, (Ptr{wlr_xdg_surface_v6}, Bool), surface, fullscreen)
end

function wlr_xdg_toplevel_v6_set_resizing(surface, resizing)
    ccall((:wlr_xdg_toplevel_v6_set_resizing, wlr_xdg_shell_v6), UInt32, (Ptr{wlr_xdg_surface_v6}, Bool), surface, resizing)
end

function wlr_xdg_surface_v6_send_close(surface)
    ccall((:wlr_xdg_surface_v6_send_close, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_surface_v6},), surface)
end

function wlr_xdg_surface_v6_surface_at(surface, sx, sy, sub_x, sub_y)
    ccall((:wlr_xdg_surface_v6_surface_at, wlr_xdg_shell_v6), Ptr{wlr_surface}, (Ptr{wlr_xdg_surface_v6}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), surface, sx, sy, sub_x, sub_y)
end

function wlr_xdg_positioner_v6_get_geometry(positioner)
    ccall((:wlr_xdg_positioner_v6_get_geometry, wlr_xdg_shell_v6), wlr_box, (Ptr{wlr_xdg_positioner_v6},), positioner)
end

function wlr_xdg_popup_v6_get_anchor_point(popup, toplevel_sx, toplevel_sy)
    ccall((:wlr_xdg_popup_v6_get_anchor_point, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_popup_v6}, Ptr{Cint}, Ptr{Cint}), popup, toplevel_sx, toplevel_sy)
end

function wlr_xdg_popup_v6_get_toplevel_coords(popup, popup_sx, popup_sy, toplevel_sx, toplevel_sy)
    ccall((:wlr_xdg_popup_v6_get_toplevel_coords, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_popup_v6}, Cint, Cint, Ptr{Cint}, Ptr{Cint}), popup, popup_sx, popup_sy, toplevel_sx, toplevel_sy)
end

function wlr_xdg_popup_v6_unconstrain_from_box(popup, toplevel_sx_box)
    ccall((:wlr_xdg_popup_v6_unconstrain_from_box, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_popup_v6}, Ptr{wlr_box}), popup, toplevel_sx_box)
end

function wlr_positioner_v6_invert_x(positioner)
    ccall((:wlr_positioner_v6_invert_x, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_positioner_v6},), positioner)
end

function wlr_positioner_v6_invert_y(positioner)
    ccall((:wlr_positioner_v6_invert_y, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_positioner_v6},), positioner)
end

function wlr_surface_is_xdg_surface_v6(surface)
    ccall((:wlr_surface_is_xdg_surface_v6, wlr_xdg_shell_v6), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_xdg_surface_v6_from_wlr_surface(surface)
    ccall((:wlr_xdg_surface_v6_from_wlr_surface, wlr_xdg_shell_v6), Ptr{wlr_xdg_surface_v6}, (Ptr{wlr_surface},), surface)
end

function wlr_xdg_surface_v6_get_geometry(surface, box)
    ccall((:wlr_xdg_surface_v6_get_geometry, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_surface_v6}, Ptr{wlr_box}), surface, box)
end

function wlr_xdg_surface_v6_for_each_surface(surface, iterator, user_data)
    ccall((:wlr_xdg_surface_v6_for_each_surface, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_surface_v6}, wlr_surface_iterator_func_t, Ptr{Cvoid}), surface, iterator, user_data)
end

function wlr_xdg_surface_v6_for_each_popup(surface, iterator, user_data)
    ccall((:wlr_xdg_surface_v6_for_each_popup, wlr_xdg_shell_v6), Cvoid, (Ptr{wlr_xdg_surface_v6}, wlr_surface_iterator_func_t, Ptr{Cvoid}), surface, iterator, user_data)
end
