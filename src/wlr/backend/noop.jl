# Julia wrapper for header: noop.h
# Automatically generated using Clang.jl


function wlr_noop_backend_create(display)
    ccall((:wlr_noop_backend_create, noop), Ptr{wlr_backend}, (Ptr{wl_display},), display)
end

function wlr_noop_add_output(backend)
    ccall((:wlr_noop_add_output, noop), Ptr{wlr_output}, (Ptr{wlr_backend},), backend)
end

function wlr_backend_is_noop(backend)
    ccall((:wlr_backend_is_noop, noop), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_output_is_noop(output)
    ccall((:wlr_output_is_noop, noop), Bool, (Ptr{wlr_output},), output)
end
