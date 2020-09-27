# Julia wrapper for header: wlr_linux_dmabuf_v1.h
# Automatically generated using Clang.jl


function wlr_dmabuf_v1_resource_is_buffer(buffer_resource)
    ccall((:wlr_dmabuf_v1_resource_is_buffer, wlr_linux_dmabuf_v1), Bool, (Ptr{wl_resource},), buffer_resource)
end

function wlr_dmabuf_v1_buffer_from_buffer_resource(buffer_resource)
    ccall((:wlr_dmabuf_v1_buffer_from_buffer_resource, wlr_linux_dmabuf_v1), Ptr{wlr_dmabuf_v1_buffer}, (Ptr{wl_resource},), buffer_resource)
end

function wlr_dmabuf_v1_buffer_from_params_resource(params_resource)
    ccall((:wlr_dmabuf_v1_buffer_from_params_resource, wlr_linux_dmabuf_v1), Ptr{wlr_dmabuf_v1_buffer}, (Ptr{wl_resource},), params_resource)
end

function wlr_linux_dmabuf_v1_create(display, renderer)
    ccall((:wlr_linux_dmabuf_v1_create, wlr_linux_dmabuf_v1), Ptr{wlr_linux_dmabuf_v1}, (Ptr{wl_display}, Ptr{wlr_renderer}), display, renderer)
end

function wlr_linux_dmabuf_v1_from_resource(resource)
    ccall((:wlr_linux_dmabuf_v1_from_resource, wlr_linux_dmabuf_v1), Ptr{wlr_linux_dmabuf_v1}, (Ptr{wl_resource},), resource)
end
