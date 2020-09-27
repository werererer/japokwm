# Julia wrapper for header: wlr_pointer.h
# Automatically generated using Clang.jl


function wlr_pointer_init(pointer, impl)
    ccall((:wlr_pointer_init, wlr_pointer), Cvoid, (Ptr{wlr_pointer}, Ptr{wlr_pointer_impl}), pointer, impl)
end

function wlr_pointer_destroy(pointer)
    ccall((:wlr_pointer_destroy, wlr_pointer), Cvoid, (Ptr{wlr_pointer},), pointer)
end
