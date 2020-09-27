# Julia wrapper for header: headless.h
# Automatically generated using Clang.jl


function wlr_headless_backend_create(display, create_renderer_func)
    ccall((:wlr_headless_backend_create, headless), Ptr{wlr_backend}, (Ptr{wl_display}, wlr_renderer_create_func_t), display, create_renderer_func)
end

function wlr_headless_backend_create_with_renderer(display, renderer)
    ccall((:wlr_headless_backend_create_with_renderer, headless), Ptr{wlr_backend}, (Ptr{wl_display}, Ptr{wlr_renderer}), display, renderer)
end

function wlr_headless_add_output(backend, width, height)
    ccall((:wlr_headless_add_output, headless), Ptr{wlr_output}, (Ptr{wlr_backend}, UInt32, UInt32), backend, width, height)
end

function wlr_headless_add_input_device(backend, type)
    ccall((:wlr_headless_add_input_device, headless), Ptr{wlr_input_device}, (Ptr{wlr_backend}, wlr_input_device_type), backend, type)
end

function wlr_backend_is_headless(backend)
    ccall((:wlr_backend_is_headless, headless), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_input_device_is_headless(device)
    ccall((:wlr_input_device_is_headless, headless), Bool, (Ptr{wlr_input_device},), device)
end

function wlr_output_is_headless(output)
    ccall((:wlr_output_is_headless, headless), Bool, (Ptr{wlr_output},), output)
end
