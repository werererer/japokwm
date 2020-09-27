# Julia wrapper for header: libinput.h
# Automatically generated using Clang.jl


function wlr_libinput_backend_create(display, session)
    ccall((:wlr_libinput_backend_create, libinput), Ptr{wlr_backend}, (Ptr{wl_display}, Ptr{wlr_session}), display, session)
end

function wlr_libinput_get_device_handle(dev)
    ccall((:wlr_libinput_get_device_handle, libinput), Ptr{libinput_device}, (Ptr{wlr_input_device},), dev)
end

function wlr_backend_is_libinput(backend)
    ccall((:wlr_backend_is_libinput, libinput), Bool, (Ptr{wlr_backend},), backend)
end

function wlr_input_device_is_libinput(device)
    ccall((:wlr_input_device_is_libinput, libinput), Bool, (Ptr{wlr_input_device},), device)
end
