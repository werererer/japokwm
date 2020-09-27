# Julia wrapper for header: multi.h
# Automatically generated using Clang.jl


function wlr_multi_backend_create(display)
    ccall((:wlr_multi_backend_create, multi), Ptr{wlr_backend}, (Ptr{wl_display},), display)
end

function wlr_multi_backend_add(multi, backend)
    ccall((:wlr_multi_backend_add, multi), Bool, (Ptr{wlr_backend}, Ptr{wlr_backend}), multi, backend)
end

function wlr_multi_backend_remove(multi, backend)
    ccall((:wlr_multi_backend_remove, multi), Cvoid, (Ptr{wlr_backend}, Ptr{wlr_backend}), multi, backend)
end

function wlr_backend_is_multi(backend)
    ccall((:wlr_backend_is_multi, multi), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_multi_is_empty(backend)
    ccall((:wlr_multi_is_empty, multi), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_multi_for_each_backend(backend, callback, data)
    ccall((:wlr_multi_for_each_backend, multi), Cvoid, (Ptr{wlr_backend}, Ptr{Cvoid}, Ptr{Cvoid}), backend, callback, data)
end
