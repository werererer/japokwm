# Julia wrapper for header: backend.h
# Automatically generated using Clang.jl


function wlr_backend_autocreate(display, create_renderer_func)
    ccall((:wlr_backend_autocreate, backend), Ptr{wlr_backend}, (Ptr{wl_display}, wlr_renderer_create_func_t), display, create_renderer_func)
end

function wlr_backend_start(backend)
    ccall((:wlr_backend_start, backend), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_backend_destroy(backend)
    ccall((:wlr_backend_destroy, backend), Cvoid, (Ptr{wlr_backend},), backend)
end

function wlr_backend_get_renderer(backend)
    ccall((:wlr_backend_get_renderer, backend), Ptr{wlr_renderer}, (Ptr{wlr_backend},), backend)
end

function wlr_backend_get_session(backend)
    ccall((:wlr_backend_get_session, backend), Ptr{wlr_session}, (Ptr{wlr_backend},), backend)
end

function wlr_backend_get_presentation_clock(backend)
    ccall((:wlr_backend_get_presentation_clock, backend), clockid_t, (Ptr{wlr_backend},), backend)
end
