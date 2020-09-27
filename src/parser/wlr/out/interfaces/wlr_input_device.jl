# Julia wrapper for header: wlr_input_device.h
# Automatically generated using Clang.jl


function wlr_input_device_init(wlr_device, type, impl, name, vendor, product)
    ccall((:wlr_input_device_init, wlr_input_device), Cvoid, (Ptr{wlr_input_device}, wlr_input_device_type, Ptr{wlr_input_device_impl}, Cstring, Cint, Cint), wlr_device, type, impl, name, vendor, product)
end

function wlr_input_device_destroy(dev)
    ccall((:wlr_input_device_destroy, wlr_input_device), Cvoid, (Ptr{wlr_input_device},), dev)
end
