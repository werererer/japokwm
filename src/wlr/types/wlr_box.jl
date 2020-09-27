# Julia wrapper for header: wlr_box.h
# Automatically generated using Clang.jl


function wlr_box_closest_point(box, x, y, dest_x, dest_y)
    ccall((:wlr_box_closest_point, wlr_box), Cvoid, (Ptr{wlr_box}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), box, x, y, dest_x, dest_y)
end

function wlr_box_intersection(dest, box_a, box_b)
    ccall((:wlr_box_intersection, wlr_box), Bool, (Ptr{wlr_box}, Ptr{wlr_box}, Ptr{wlr_box}), dest, box_a, box_b)
end

function wlr_box_contains_point(box, x, y)
    ccall((:wlr_box_contains_point, wlr_box), Bool, (Ptr{wlr_box}, Cdouble, Cdouble), box, x, y)
end

function wlr_box_empty(box)
    ccall((:wlr_box_empty, wlr_box), Bool, (Ptr{wlr_box},), box)
end

function wlr_box_transform(dest, box, transform, width, height)
    ccall((:wlr_box_transform, wlr_box), Cvoid, (Ptr{wlr_box}, Ptr{wlr_box}, wl_output_transform, Cint, Cint), dest, box, transform, width, height)
end

function wlr_box_rotated_bounds(dest, box, rotation)
    ccall((:wlr_box_rotated_bounds, wlr_box), Cvoid, (Ptr{wlr_box}, Ptr{wlr_box}, Cfloat), dest, box, rotation)
end

function wlr_box_from_pixman_box32(dest, box)
    ccall((:wlr_box_from_pixman_box32, wlr_box), Cvoid, (Ptr{wlr_box}, pixman_box32_t), dest, box)
end
