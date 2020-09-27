# Julia wrapper for header: interface.h
# Automatically generated using Clang.jl


function wlr_renderer_init(renderer, impl)
    ccall((:wlr_renderer_init, interface), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_renderer_impl}), renderer, impl)
end

function wlr_texture_init(texture, impl, width, height)
    ccall((:wlr_texture_init, interface), Cvoid, (Ptr{wlr_texture}, Ptr{wlr_texture_impl}, UInt32, UInt32), texture, impl, width, height)
end
formats::Ptr{Cvoid}
    preferred_read_format::Ptr{Cvoid}
    read_pixels::Ptr{Cvoid}
    texture_from_pixels::Ptr{Cvoid}
    texture_from_wl_drm::Ptr{Cvoid}
    texture_from_dmabuf::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    init_wl_display::Ptr{Cvoid}
    blit_dmabuf::Ptr{Cvoid}
end

struct wlr_texture_impl
    is_opaque::Ptr{Cvoid}
    write_pixels::Ptr{Cvoid}
    to_dmabuf::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
end
