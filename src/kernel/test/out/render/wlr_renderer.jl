# Julia wrapper for header: wlr_renderer.h
# Automatically generated using Clang.jl


function wlr_renderer_autocreate(egl, platform, remote_display, config_attribs, visual_id)
    ccall((:wlr_renderer_autocreate, wlr_renderer), Ptr{wlr_renderer}, (Ptr{wlr_egl}, EGLenum, Ptr{Cvoid}, Ptr{EGLint}, EGLint), egl, platform, remote_display, config_attribs, visual_id)
end

function wlr_renderer_begin(r, width, height)
    ccall((:wlr_renderer_begin, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Cint, Cint), r, width, height)
end

function wlr_renderer_end(r)
    ccall((:wlr_renderer_end, wlr_renderer), Cvoid, (Ptr{wlr_renderer},), r)
end

function wlr_renderer_clear(r, color)
    ccall((:wlr_renderer_clear, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{Cfloat}), r, color)
end

function wlr_renderer_scissor(r, box)
    ccall((:wlr_renderer_scissor, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_box}), r, box)
end

function wlr_render_texture(r, texture, projection, x, y, alpha)
    ccall((:wlr_render_texture, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_texture}, Ptr{Cfloat}, Cint, Cint, Cfloat), r, texture, projection, x, y, alpha)
end

function wlr_render_texture_with_matrix(r, texture, matrix, alpha)
    ccall((:wlr_render_texture_with_matrix, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_texture}, Ptr{Cfloat}, Cfloat), r, texture, matrix, alpha)
end

function wlr_render_subtexture_with_matrix(r, texture, box, matrix, alpha)
    ccall((:wlr_render_subtexture_with_matrix, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_texture}, Ptr{wlr_fbox}, Ptr{Cfloat}, Cfloat), r, texture, box, matrix, alpha)
end

function wlr_render_rect(r, box, color, projection)
    ccall((:wlr_render_rect, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_box}, Ptr{Cfloat}, Ptr{Cfloat}), r, box, color, projection)
end

function wlr_render_quad_with_matrix(r, color, matrix)
    ccall((:wlr_render_quad_with_matrix, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{Cfloat}, Ptr{Cfloat}), r, color, matrix)
end

function wlr_render_ellipse(r, box, color, projection)
    ccall((:wlr_render_ellipse, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_box}, Ptr{Cfloat}, Ptr{Cfloat}), r, box, color, projection)
end

function wlr_render_ellipse_with_matrix(r, color, matrix)
    ccall((:wlr_render_ellipse_with_matrix, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{Cfloat}, Ptr{Cfloat}), r, color, matrix)
end

function wlr_renderer_get_formats(r, len)
    ccall((:wlr_renderer_get_formats, wlr_renderer), Ptr{wl_shm_format}, (Ptr{wlr_renderer}, Ptr{Csize_t}), r, len)
end

function wlr_renderer_resource_is_wl_drm_buffer(renderer, buffer)
    ccall((:wlr_renderer_resource_is_wl_drm_buffer, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wl_resource}), renderer, buffer)
end

function wlr_renderer_wl_drm_buffer_get_size(renderer, buffer, width, height)
    ccall((:wlr_renderer_wl_drm_buffer_get_size, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wl_resource}, Ptr{Cint}, Ptr{Cint}), renderer, buffer, width, height)
end

function wlr_renderer_get_dmabuf_formats(renderer)
    ccall((:wlr_renderer_get_dmabuf_formats, wlr_renderer), Ptr{wlr_drm_format_set}, (Ptr{wlr_renderer},), renderer)
end

function wlr_renderer_read_pixels(r, fmt, flags, stride, width, height, src_x, src_y, dst_x, dst_y, data)
    ccall((:wlr_renderer_read_pixels, wlr_renderer), Bool, (Ptr{wlr_renderer}, wl_shm_format, Ptr{UInt32}, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, Ptr{Cvoid}), r, fmt, flags, stride, width, height, src_x, src_y, dst_x, dst_y, data)
end

function wlr_renderer_blit_dmabuf(r, dst, src)
    ccall((:wlr_renderer_blit_dmabuf, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_dmabuf_attributes}, Ptr{wlr_dmabuf_attributes}), r, dst, src)
end

function wlr_renderer_format_supported(r, fmt)
    ccall((:wlr_renderer_format_supported, wlr_renderer), Bool, (Ptr{wlr_renderer}, wl_shm_format), r, fmt)
end

function wlr_renderer_init_wl_display(r, wl_display)
    ccall((:wlr_renderer_init_wl_display, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wl_display}), r, wl_display)
end

function wlr_renderer_destroy(renderer)
    ccall((:wlr_renderer_destroy, wlr_renderer), Cvoid, (Ptr{wlr_renderer},), renderer)
end
