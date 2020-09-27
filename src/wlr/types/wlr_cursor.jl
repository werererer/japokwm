# Julia wrapper for header: wlr_cursor.h
# Automatically generated using Clang.jl


function wlr_cursor_create()
    ccall((:wlr_cursor_create, wlr_cursor), Ptr{wlr_cursor}, ())
end

function wlr_cursor_destroy(cur)
    ccall((:wlr_cursor_destroy, wlr_cursor), Cvoid, (Ptr{wlr_cursor},), cur)
end

function wlr_cursor_warp(cur, dev, lx, ly)
    ccall((:wlr_cursor_warp, wlr_cursor), Bool, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Cdouble, Cdouble), cur, dev, lx, ly)
end

function wlr_cursor_absolute_to_layout_coords(cur, dev, x, y, lx, ly)
    ccall((:wlr_cursor_absolute_to_layout_coords, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), cur, dev, x, y, lx, ly)
end

function wlr_cursor_warp_closest(cur, dev, x, y)
    ccall((:wlr_cursor_warp_closest, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Cdouble, Cdouble), cur, dev, x, y)
end

function wlr_cursor_warp_absolute(cur, dev, x, y)
    ccall((:wlr_cursor_warp_absolute, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Cdouble, Cdouble), cur, dev, x, y)
end

function wlr_cursor_move(cur, dev, delta_x, delta_y)
    ccall((:wlr_cursor_move, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Cdouble, Cdouble), cur, dev, delta_x, delta_y)
end

function wlr_cursor_set_image(cur, pixels, stride, width, height, hotspot_x, hotspot_y, scale)
    ccall((:wlr_cursor_set_image, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{UInt8}, Int32, UInt32, UInt32, Int32, Int32, Cfloat), cur, pixels, stride, width, height, hotspot_x, hotspot_y, scale)
end

function wlr_cursor_set_surface(cur, surface, hotspot_x, hotspot_y)
    ccall((:wlr_cursor_set_surface, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_surface}, Int32, Int32), cur, surface, hotspot_x, hotspot_y)
end

function wlr_cursor_attach_input_device(cur, dev)
    ccall((:wlr_cursor_attach_input_device, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}), cur, dev)
end

function wlr_cursor_detach_input_device(cur, dev)
    ccall((:wlr_cursor_detach_input_device, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}), cur, dev)
end

function wlr_cursor_attach_output_layout(cur, l)
    ccall((:wlr_cursor_attach_output_layout, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_output_layout}), cur, l)
end

function wlr_cursor_map_to_output(cur, output)
    ccall((:wlr_cursor_map_to_output, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_output}), cur, output)
end

function wlr_cursor_map_input_to_output(cur, dev, output)
    ccall((:wlr_cursor_map_input_to_output, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Ptr{wlr_output}), cur, dev, output)
end

function wlr_cursor_map_to_region(cur, box)
    ccall((:wlr_cursor_map_to_region, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_box}), cur, box)
end

function wlr_cursor_map_input_to_region(cur, dev, box)
    ccall((:wlr_cursor_map_input_to_region, wlr_cursor), Cvoid, (Ptr{wlr_cursor}, Ptr{wlr_input_device}, Ptr{wlr_box}), cur, dev, box)
end
