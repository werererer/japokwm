# Julia wrapper for header: x11.h
# Automatically generated using Clang.jl


function wlr_x11_backend_create(display, x11_display, create_renderer_func)
    ccall((:wlr_x11_backend_create, x11), Ptr{wlr_backend}, (Ptr{wl_display}, Cstring, wlr_renderer_create_func_t), display, x11_display, create_renderer_func)
end

function wlr_x11_output_create(backend)
    ccall((:wlr_x11_output_create, x11), Ptr{wlr_output}, (Ptr{wlr_backend},), backend)
end

function wlr_backend_is_x11(backend)
    ccall((:wlr_backend_is_x11, x11), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_input_device_is_x11(device)
    ccall((:wlr_input_device_is_x11, x11), Bool, (Ptr{wlr_input_device},), device)
end

function wlr_output_is_x11(output)
    ccall((:wlr_output_is_x11, x11), Bool, (Ptr{wlr_output},), output)
end

function wlr_x11_output_set_title(output, title)
    ccall((:wlr_x11_output_set_title, x11), Cvoid, (Ptr{wlr_output}, Cstring), output, title)
end
