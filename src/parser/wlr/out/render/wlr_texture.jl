# Julia wrapper for header: wlr_texture.h
# Automatically generated using Clang.jl


function wlr_texture_from_pixels(renderer, wl_fmt, stride, width, height, data)
    ccall((:wlr_texture_from_pixels, wlr_texture), Ptr{wlr_texture}, (Ptr{wlr_renderer}, wl_shm_format, UInt32, UInt32, UInt32, Ptr{Cvoid}), renderer, wl_fmt, stride, width, height, data)
end

function wlr_texture_from_wl_drm(renderer, data)
    ccall((:wlr_texture_from_wl_drm, wlr_texture), Ptr{wlr_texture}, (Ptr{wlr_renderer}, Ptr{wl_resource}), renderer, data)
end

function wlr_texture_from_dmabuf(renderer, attribs)
    ccall((:wlr_texture_from_dmabuf, wlr_texture), Ptr{wlr_texture}, (Ptr{wlr_renderer}, Ptr{wlr_dmabuf_attributes}), renderer, attribs)
end

function wlr_texture_get_size(texture, width, height)
    ccall((:wlr_texture_get_size, wlr_texture), Cvoid, (Ptr{wlr_texture}, Ptr{Cint}, Ptr{Cint}), texture, width, height)
end

function wlr_texture_is_opaque(texture)
    ccall((:wlr_texture_is_opaque, wlr_texture), Bool, (Ptr{wlr_texture},), texture)
end

function wlr_texture_write_pixels(texture, stride, width, height, src_x, src_y, dst_x, dst_y, data)
    ccall((:wlr_texture_write_pixels, wlr_texture), Bool, (Ptr{wlr_texture}, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, Ptr{Cvoid}), texture, stride, width, height, src_x, src_y, dst_x, dst_y, data)
end

function wlr_texture_to_dmabuf(texture, attribs)
    ccall((:wlr_texture_to_dmabuf, wlr_texture), Bool, (Ptr{wlr_texture}, Ptr{wlr_dmabuf_attributes}), texture, attribs)
end

function wlr_texture_destroy(texture)
    ccall((:wlr_texture_destroy, wlr_texture), Cvoid, (Ptr{wlr_texture},), texture)
end
