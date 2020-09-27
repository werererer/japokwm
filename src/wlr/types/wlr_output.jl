# Julia wrapper for header: wlr_output.h
# Automatically generated using Clang.jl


function wlr_output_enable(output, enable)
    ccall((:wlr_output_enable, wlr_output), Cvoid, (Ptr{wlr_output}, Bool), output, enable)
end

function wlr_output_create_global(output)
    ccall((:wlr_output_create_global, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_destroy_global(output)
    ccall((:wlr_output_destroy_global, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_preferred_mode(output)
    ccall((:wlr_output_preferred_mode, wlr_output), Ptr{wlr_output_mode}, (Ptr{wlr_output},), output)
end

function wlr_output_set_mode(output, mode)
    ccall((:wlr_output_set_mode, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{wlr_output_mode}), output, mode)
end

function wlr_output_set_custom_mode(output, width, height, refresh)
    ccall((:wlr_output_set_custom_mode, wlr_output), Cvoid, (Ptr{wlr_output}, Int32, Int32, Int32), output, width, height, refresh)
end

function wlr_output_set_transform(output, transform)
    ccall((:wlr_output_set_transform, wlr_output), Cvoid, (Ptr{wlr_output}, wl_output_transform), output, transform)
end

function wlr_output_enable_adaptive_sync(output, enabled)
    ccall((:wlr_output_enable_adaptive_sync, wlr_output), Cvoid, (Ptr{wlr_output}, Bool), output, enabled)
end

function wlr_output_set_scale(output, scale)
    ccall((:wlr_output_set_scale, wlr_output), Cvoid, (Ptr{wlr_output}, Cfloat), output, scale)
end

function wlr_output_set_subpixel(output, subpixel)
    ccall((:wlr_output_set_subpixel, wlr_output), Cvoid, (Ptr{wlr_output}, wl_output_subpixel), output, subpixel)
end

function wlr_output_set_description(output, desc)
    ccall((:wlr_output_set_description, wlr_output), Cvoid, (Ptr{wlr_output}, Cstring), output, desc)
end

function wlr_output_schedule_done(output)
    ccall((:wlr_output_schedule_done, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_destroy(output)
    ccall((:wlr_output_destroy, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_transformed_resolution(output, width, height)
    ccall((:wlr_output_transformed_resolution, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{Cint}, Ptr{Cint}), output, width, height)
end

function wlr_output_effective_resolution(output, width, height)
    ccall((:wlr_output_effective_resolution, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{Cint}, Ptr{Cint}), output, width, height)
end

function wlr_output_attach_render(output, buffer_age)
    ccall((:wlr_output_attach_render, wlr_output), Bool, (Ptr{wlr_output}, Ptr{Cint}), output, buffer_age)
end

function wlr_output_attach_buffer(output, buffer)
    ccall((:wlr_output_attach_buffer, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{wlr_buffer}), output, buffer)
end

function wlr_output_preferred_read_format(output, fmt)
    ccall((:wlr_output_preferred_read_format, wlr_output), Bool, (Ptr{wlr_output}, Ptr{wl_shm_format}), output, fmt)
end

function wlr_output_set_damage(output, damage)
    ccall((:wlr_output_set_damage, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{pixman_region32_t}), output, damage)
end

function wlr_output_test(output)
    ccall((:wlr_output_test, wlr_output), Bool, (Ptr{wlr_output},), output)
end

function wlr_output_commit(output)
    ccall((:wlr_output_commit, wlr_output), Bool, (Ptr{wlr_output},), output)
end

function wlr_output_rollback(output)
    ccall((:wlr_output_rollback, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_schedule_frame(output)
    ccall((:wlr_output_schedule_frame, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_get_gamma_size(output)
    ccall((:wlr_output_get_gamma_size, wlr_output), Csize_t, (Ptr{wlr_output},), output)
end

function wlr_output_set_gamma(output, size, r, g, b)
    ccall((:wlr_output_set_gamma, wlr_output), Cvoid, (Ptr{wlr_output}, Csize_t, Ptr{UInt16}, Ptr{UInt16}, Ptr{UInt16}), output, size, r, g, b)
end

function wlr_output_export_dmabuf(output, attribs)
    ccall((:wlr_output_export_dmabuf, wlr_output), Bool, (Ptr{wlr_output}, Ptr{wlr_dmabuf_attributes}), output, attribs)
end

function wlr_output_from_resource(resource)
    ccall((:wlr_output_from_resource, wlr_output), Ptr{wlr_output}, (Ptr{wl_resource},), resource)
end

function wlr_output_lock_attach_render(output, lock)
    ccall((:wlr_output_lock_attach_render, wlr_output), Cvoid, (Ptr{wlr_output}, Bool), output, lock)
end

function wlr_output_lock_software_cursors(output, lock)
    ccall((:wlr_output_lock_software_cursors, wlr_output), Cvoid, (Ptr{wlr_output}, Bool), output, lock)
end

function wlr_output_render_software_cursors(output, damage)
    ccall((:wlr_output_render_software_cursors, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{pixman_region32_t}), output, damage)
end

function wlr_output_cursor_create(output)
    ccall((:wlr_output_cursor_create, wlr_output), Ptr{wlr_output_cursor}, (Ptr{wlr_output},), output)
end

function wlr_output_cursor_set_image(cursor, pixels, stride, width, height, hotspot_x, hotspot_y)
    ccall((:wlr_output_cursor_set_image, wlr_output), Bool, (Ptr{wlr_output_cursor}, Ptr{UInt8}, Int32, UInt32, UInt32, Int32, Int32), cursor, pixels, stride, width, height, hotspot_x, hotspot_y)
end

function wlr_output_cursor_set_surface(cursor, surface, hotspot_x, hotspot_y)
    ccall((:wlr_output_cursor_set_surface, wlr_output), Cvoid, (Ptr{wlr_output_cursor}, Ptr{wlr_surface}, Int32, Int32), cursor, surface, hotspot_x, hotspot_y)
end

function wlr_output_cursor_move(cursor, x, y)
    ccall((:wlr_output_cursor_move, wlr_output), Bool, (Ptr{wlr_output_cursor}, Cdouble, Cdouble), cursor, x, y)
end

function wlr_output_cursor_destroy(cursor)
    ccall((:wlr_output_cursor_destroy, wlr_output), Cvoid, (Ptr{wlr_output_cursor},), cursor)
end

function wlr_output_transform_invert(tr)
    ccall((:wlr_output_transform_invert, wlr_output), wl_output_transform, (wl_output_transform,), tr)
end

function wlr_output_transform_compose(tr_a, tr_b)
    ccall((:wlr_output_transform_compose, wlr_output), wl_output_transform, (wl_output_transform, wl_output_transform), tr_a, tr_b)
end
