# Julia wrapper for header: wlr_buffer.h
# Automatically generated using Clang.jl


function wlr_buffer_init(buffer, impl, width, height)
    ccall((:wlr_buffer_init, wlr_buffer), Cvoid, (Ptr{wlr_buffer}, Ptr{wlr_buffer_impl}, Cint, Cint), buffer, impl, width, height)
end

function wlr_buffer_drop(buffer)
    ccall((:wlr_buffer_drop, wlr_buffer), Cvoid, (Ptr{wlr_buffer},), buffer)
end

function wlr_buffer_lock(buffer)
    ccall((:wlr_buffer_lock, wlr_buffer), Ptr{wlr_buffer}, (Ptr{wlr_buffer},), buffer)
end

function wlr_buffer_unlock(buffer)
    ccall((:wlr_buffer_unlock, wlr_buffer), Cvoid, (Ptr{wlr_buffer},), buffer)
end

function wlr_buffer_get_dmabuf(buffer, attribs)
    ccall((:wlr_buffer_get_dmabuf, wlr_buffer), Bool, (Ptr{wlr_buffer}, Ptr{wlr_dmabuf_attributes}), buffer, attribs)
end

function wlr_resource_is_buffer(resource)
    ccall((:wlr_resource_is_buffer, wlr_buffer), Bool, (Ptr{wl_resource},), resource)
end

function wlr_resource_get_buffer_size(resource, renderer, width, height)
    ccall((:wlr_resource_get_buffer_size, wlr_buffer), Bool, (Ptr{wl_resource}, Ptr{wlr_renderer}, Ptr{Cint}, Ptr{Cint}), resource, renderer, width, height)
end

function wlr_client_buffer_import(renderer, resource)
    ccall((:wlr_client_buffer_import, wlr_buffer), Ptr{wlr_client_buffer}, (Ptr{wlr_renderer}, Ptr{wl_resource}), renderer, resource)
end

function wlr_client_buffer_apply_damage(buffer, resource, damage)
    ccall((:wlr_client_buffer_apply_damage, wlr_buffer), Ptr{wlr_client_buffer}, (Ptr{wlr_client_buffer}, Ptr{wl_resource}, Ptr{pixman_region32_t}), buffer, resource, damage)
end
