# Julia wrapper for header: drm.h
# Automatically generated using Clang.jl


function wlr_drm_backend_create(display, session, gpu_fd, parent, create_renderer_func)
    ccall((:wlr_drm_backend_create, drm), Ptr{wlr_backend}, (Ptr{wl_display}, Ptr{wlr_session}, Cint, Ptr{wlr_backend}, wlr_renderer_create_func_t), display, session, gpu_fd, parent, create_renderer_func)
end

function wlr_backend_is_drm(backend)
    ccall((:wlr_backend_is_drm, drm), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_output_is_drm(output)
    ccall((:wlr_output_is_drm, drm), Bool, (Ptr{wlr_output},), output)
end

function wlr_drm_connector_add_mode(output, mode)
    ccall((:wlr_drm_connector_add_mode, drm), Ptr{wlr_output_mode}, (Ptr{wlr_output}, Ptr{drmModeModeInfo}), output, mode)
end
