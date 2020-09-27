# Julia wrapper for header: interface.h
# Automatically generated using Clang.jl


function wlr_backend_init(backend, impl)
    ccall((:wlr_backend_init, interface), Cvoid, (Ptr{wlr_backend}, Ptr{wlr_backend_impl}), backend, impl)
end
