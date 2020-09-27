# Julia wrapper for header: egl.h
# Automatically generated using Clang.jl


function wlr_egl_init(egl, platform, remote_display, config_attribs, visual_id)
    ccall((:wlr_egl_init, egl), Bool, (Ptr{wlr_egl}, EGLenum, Ptr{Cvoid}, Ptr{EGLint}, EGLint), egl, platform, remote_display, config_attribs, visual_id)
end

function wlr_egl_finish(egl)
    ccall((:wlr_egl_finish, egl), Cvoid, (Ptr{wlr_egl},), egl)
end

function wlr_egl_bind_display(egl, local_display)
    ccall((:wlr_egl_bind_display, egl), Bool, (Ptr{wlr_egl}, Ptr{wl_display}), egl, local_display)
end

function wlr_egl_create_surface(egl, window)
    ccall((:wlr_egl_create_surface, egl), EGLSurface, (Ptr{wlr_egl}, Ptr{Cvoid}), egl, window)
end

function wlr_egl_create_image_from_wl_drm(egl, data, fmt, width, height, inverted_y)
    ccall((:wlr_egl_create_image_from_wl_drm, egl), EGLImageKHR, (Ptr{wlr_egl}, Ptr{wl_resource}, Ptr{EGLint}, Ptr{Cint}, Ptr{Cint}, Ptr{Bool}), egl, data, fmt, width, height, inverted_y)
end

function wlr_egl_create_image_from_dmabuf(egl, attributes, external_only)
    ccall((:wlr_egl_create_image_from_dmabuf, egl), EGLImageKHR, (Ptr{wlr_egl}, Ptr{wlr_dmabuf_attributes}, Ptr{Bool}), egl, attributes, external_only)
end

function wlr_egl_get_dmabuf_formats(egl)
    ccall((:wlr_egl_get_dmabuf_formats, egl), Ptr{wlr_drm_format_set}, (Ptr{wlr_egl},), egl)
end

function wlr_egl_export_image_to_dmabuf(egl, image, width, height, flags, attribs)
    ccall((:wlr_egl_export_image_to_dmabuf, egl), Bool, (Ptr{wlr_egl}, EGLImageKHR, Int32, Int32, UInt32, Ptr{wlr_dmabuf_attributes}), egl, image, width, height, flags, attribs)
end

function wlr_egl_destroy_image(egl, image)
    ccall((:wlr_egl_destroy_image, egl), Bool, (Ptr{wlr_egl}, EGLImageKHR), egl, image)
end

function wlr_egl_make_current(egl, surface, buffer_age)
    ccall((:wlr_egl_make_current, egl), Bool, (Ptr{wlr_egl}, EGLSurface, Ptr{Cint}), egl, surface, buffer_age)
end

function wlr_egl_unset_current(egl)
    ccall((:wlr_egl_unset_current, egl), Bool, (Ptr{wlr_egl},), egl)
end

function wlr_egl_is_current(egl)
    ccall((:wlr_egl_is_current, egl), Bool, (Ptr{wlr_egl},), egl)
end

function wlr_egl_save_context(context)
    ccall((:wlr_egl_save_context, egl), Cvoid, (Ptr{wlr_egl_context},), context)
end

function wlr_egl_restore_context(context)
    ccall((:wlr_egl_restore_context, egl), Bool, (Ptr{wlr_egl_context},), context)
end

function wlr_egl_swap_buffers(egl, surface, damage)
    ccall((:wlr_egl_swap_buffers, egl), Bool, (Ptr{wlr_egl}, EGLSurface, Ptr{pixman_region32_t}), egl, surface, damage)
end

function wlr_egl_destroy_surface(egl, surface)
    ccall((:wlr_egl_destroy_surface, egl), Bool, (Ptr{wlr_egl}, EGLSurface), egl, surface)
end
