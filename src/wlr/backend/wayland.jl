# Julia wrapper for header: wayland.h
# Automatically generated using Clang.jl


function wlr_wl_backend_create(display, remote, create_renderer_func)
    ccall((:wlr_wl_backend_create, wayland), Ptr{wlr_backend}, (Ptr{wl_display}, Cstring, wlr_renderer_create_func_t), display, remote, create_renderer_func)
end

function wlr_wl_backend_get_remote_display(backend)
    ccall((:wlr_wl_backend_get_remote_display, wayland), Ptr{wl_display}, (Ptr{wlr_backend},), backend)
end

function wlr_wl_output_create(backend)
    ccall((:wlr_wl_output_create, wayland), Ptr{wlr_output}, (Ptr{wlr_backend},), backend)
end

function wlr_backend_is_wl(backend)
    ccall((:wlr_backend_is_wl, wayland), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_input_device_is_wl(device)
    ccall((:wlr_input_device_is_wl, wayland), Bool, (Ptr{wlr_input_device},), device)
end

function wlr_output_is_wl(output)
    ccall((:wlr_output_is_wl, wayland), Bool, (Ptr{wlr_output},), output)
end

function wlr_wl_output_set_title(output, title)
    ccall((:wlr_wl_output_set_title, wayland), Cvoid, (Ptr{wlr_output}, Cstring), output, title)
end

function wlr_wl_output_get_surface(output)
    ccall((:wlr_wl_output_get_surface, wayland), Ptr{wl_surface}, (Ptr{wlr_output},), output)
end

function wlr_wl_input_device_get_seat(dev)
    ccall((:wlr_wl_input_device_get_seat, wayland), Ptr{wl_seat}, (Ptr{wlr_input_device},), dev)
end
