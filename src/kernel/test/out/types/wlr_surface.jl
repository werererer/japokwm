# Julia wrapper for header: wlr_surface.h
# Automatically generated using Clang.jl


function wlr_surface_create(client, version, id, renderer, resource_list)
    ccall((:wlr_surface_create, wlr_surface), Ptr{wlr_surface}, (Ptr{wl_client}, UInt32, UInt32, Ptr{wlr_renderer}, Ptr{wl_list}), client, version, id, renderer, resource_list)
end

function wlr_surface_set_role(surface, role, role_data, error_resource, error_code)
    ccall((:wlr_surface_set_role, wlr_surface), Bool, (Ptr{wlr_surface}, Ptr{wlr_surface_role}, Ptr{Cvoid}, Ptr{wl_resource}, UInt32), surface, role, role_data, error_resource, error_code)
end

function wlr_surface_has_buffer(surface)
    ccall((:wlr_surface_has_buffer, wlr_surface), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_surface_get_texture(surface)
    ccall((:wlr_surface_get_texture, wlr_surface), Ptr{wlr_texture}, (Ptr{wlr_surface},), surface)
end

function wlr_subsurface_create(surface, parent, version, id, resource_list)
    ccall((:wlr_subsurface_create, wlr_surface), Ptr{wlr_subsurface}, (Ptr{wlr_surface}, Ptr{wlr_surface}, UInt32, UInt32, Ptr{wl_list}), surface, parent, version, id, resource_list)
end

function wlr_surface_get_root_surface(surface)
    ccall((:wlr_surface_get_root_surface, wlr_surface), Ptr{wlr_surface}, (Ptr{wlr_surface},), surface)
end

function wlr_surface_point_accepts_input(surface, sx, sy)
    ccall((:wlr_surface_point_accepts_input, wlr_surface), Bool, (Ptr{wlr_surface}, Cdouble, Cdouble), surface, sx, sy)
end

function wlr_surface_surface_at(surface, sx, sy, sub_x, sub_y)
    ccall((:wlr_surface_surface_at, wlr_surface), Ptr{wlr_surface}, (Ptr{wlr_surface}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), surface, sx, sy, sub_x, sub_y)
end

function wlr_surface_send_enter(surface, output)
    ccall((:wlr_surface_send_enter, wlr_surface), Cvoid, (Ptr{wlr_surface}, Ptr{wlr_output}), surface, output)
end

function wlr_surface_send_leave(surface, output)
    ccall((:wlr_surface_send_leave, wlr_surface), Cvoid, (Ptr{wlr_surface}, Ptr{wlr_output}), surface, output)
end

function wlr_surface_send_frame_done(surface, when)
    ccall((:wlr_surface_send_frame_done, wlr_surface), Cvoid, (Ptr{wlr_surface}, Ptr{timespec}), surface, when)
end

function wlr_surface_get_extends(surface, box)
    ccall((:wlr_surface_get_extends, wlr_surface), Cvoid, (Ptr{wlr_surface}, Ptr{wlr_box}), surface, box)
end

function wlr_surface_from_resource(resource)
    ccall((:wlr_surface_from_resource, wlr_surface), Ptr{wlr_surface}, (Ptr{wl_resource},), resource)
end

function wlr_surface_for_each_surface(surface, iterator, user_data)
    ccall((:wlr_surface_for_each_surface, wlr_surface), Cvoid, (Ptr{wlr_surface}, wlr_surface_iterator_func_t, Ptr{Cvoid}), surface, iterator, user_data)
end

function wlr_surface_get_effective_damage(surface, damage)
    ccall((:wlr_surface_get_effective_damage, wlr_surface), Cvoid, (Ptr{wlr_surface}, Ptr{pixman_region32_t}), surface, damage)
end

function wlr_surface_get_buffer_source_box(surface, box)
    ccall((:wlr_surface_get_buffer_source_box, wlr_surface), Cvoid, (Ptr{wlr_surface}, Ptr{wlr_fbox}), surface, box)
end
