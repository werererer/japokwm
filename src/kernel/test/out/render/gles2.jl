# Julia wrapper for header: gles2.h
# Automatically generated using Clang.jl


function wlr_gles2_renderer_create(egl)
    ccall((:wlr_gles2_renderer_create, gles2), Ptr{wlr_renderer}, (Ptr{wlr_egl},), egl)
end

function wlr_gles2_renderer_get_egl(renderer)
    ccall((:wlr_gles2_renderer_get_egl, gles2), Ptr{wlr_egl}, (Ptr{wlr_renderer},), renderer)
end

function wlr_gles2_renderer_check_ext(renderer, ext)
    ccall((:wlr_gles2_renderer_check_ext, gles2), Bool, (Ptr{wlr_renderer}, Cstring), renderer, ext)
end

function wlr_gles2_texture_from_pixels(egl, wl_fmt, stride, width, height, data)
    ccall((:wlr_gles2_texture_from_pixels, gles2), Ptr{wlr_texture}, (Ptr{wlr_egl}, wl_shm_format, UInt32, UInt32, UInt32, Ptr{Cvoid}), egl, wl_fmt, stride, width, height, data)
end

function wlr_gles2_texture_from_wl_drm(egl, data)
    ccall((:wlr_gles2_texture_from_wl_drm, gles2), Ptr{wlr_texture}, (Ptr{wlr_egl}, Ptr{wl_resource}), egl, data)
end

function wlr_gles2_texture_from_dmabuf(egl, attribs)
    ccall((:wlr_gles2_texture_from_dmabuf, gles2), Ptr{wlr_texture}, (Ptr{wlr_egl}, Ptr{wlr_dmabuf_attributes}), egl, attribs)
end

function wlr_texture_is_gles2(texture)
    ccall((:wlr_texture_is_gles2, gles2), Bool, (Ptr{wlr_texture},), texture)
end

function wlr_gles2_texture_get_attribs(texture, attribs)
    ccall((:wlr_gles2_texture_get_attribs, gles2), Cvoid, (Ptr{wlr_texture}, Ptr{wlr_gles2_texture_attribs}), texture, attribs)
end
t wlr_gles2_texture_attribs
    target::GLenum
    tex::GLuint
    inverted_y::Bool
    has_alpha::Bool
end
