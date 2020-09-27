# Julia wrapper for header: wlr_output_layout.h
# Automatically generated using Clang.jl


function wlr_output_layout_create()
    ccall((:wlr_output_layout_create, wlr_output_layout), Ptr{wlr_output_layout}, ())
end

function wlr_output_layout_destroy(layout)
    ccall((:wlr_output_layout_destroy, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout},), layout)
end

function wlr_output_layout_get(layout, reference)
    ccall((:wlr_output_layout_get, wlr_output_layout), Ptr{wlr_output_layout_output}, (Ptr{wlr_output_layout}, Ptr{wlr_output}), layout, reference)
end

function wlr_output_layout_output_at(layout, lx, ly)
    ccall((:wlr_output_layout_output_at, wlr_output_layout), Ptr{wlr_output}, (Ptr{wlr_output_layout}, Cdouble, Cdouble), layout, lx, ly)
end

function wlr_output_layout_add(layout, output, lx, ly)
    ccall((:wlr_output_layout_add, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout}, Ptr{wlr_output}, Cint, Cint), layout, output, lx, ly)
end

function wlr_output_layout_move(layout, output, lx, ly)
    ccall((:wlr_output_layout_move, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout}, Ptr{wlr_output}, Cint, Cint), layout, output, lx, ly)
end

function wlr_output_layout_remove(layout, output)
    ccall((:wlr_output_layout_remove, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout}, Ptr{wlr_output}), layout, output)
end

function wlr_output_layout_output_coords(layout, reference, lx, ly)
    ccall((:wlr_output_layout_output_coords, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout}, Ptr{wlr_output}, Ptr{Cdouble}, Ptr{Cdouble}), layout, reference, lx, ly)
end

function wlr_output_layout_contains_point(layout, reference, lx, ly)
    ccall((:wlr_output_layout_contains_point, wlr_output_layout), Bool, (Ptr{wlr_output_layout}, Ptr{wlr_output}, Cint, Cint), layout, reference, lx, ly)
end

function wlr_output_layout_intersects(layout, reference, target_lbox)
    ccall((:wlr_output_layout_intersects, wlr_output_layout), Bool, (Ptr{wlr_output_layout}, Ptr{wlr_output}, Ptr{wlr_box}), layout, reference, target_lbox)
end

function wlr_output_layout_closest_point(layout, reference, lx, ly, dest_lx, dest_ly)
    ccall((:wlr_output_layout_closest_point, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout}, Ptr{wlr_output}, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), layout, reference, lx, ly, dest_lx, dest_ly)
end

function wlr_output_layout_get_box(layout, reference)
    ccall((:wlr_output_layout_get_box, wlr_output_layout), Ptr{wlr_box}, (Ptr{wlr_output_layout}, Ptr{wlr_output}), layout, reference)
end

function wlr_output_layout_add_auto(layout, output)
    ccall((:wlr_output_layout_add_auto, wlr_output_layout), Cvoid, (Ptr{wlr_output_layout}, Ptr{wlr_output}), layout, output)
end

function wlr_output_layout_get_center_output(layout)
    ccall((:wlr_output_layout_get_center_output, wlr_output_layout), Ptr{wlr_output}, (Ptr{wlr_output_layout},), layout)
end

function wlr_output_layout_adjacent_output(layout, direction, reference, ref_lx, ref_ly)
    ccall((:wlr_output_layout_adjacent_output, wlr_output_layout), Ptr{wlr_output}, (Ptr{wlr_output_layout}, wlr_direction, Ptr{wlr_output}, Cdouble, Cdouble), layout, direction, reference, ref_lx, ref_ly)
end

function wlr_output_layout_farthest_output(layout, direction, reference, ref_lx, ref_ly)
    ccall((:wlr_output_layout_farthest_output, wlr_output_layout), Ptr{wlr_output}, (Ptr{wlr_output_layout}, wlr_direction, Ptr{wlr_output}, Cdouble, Cdouble), layout, direction, reference, ref_lx, ref_ly)
end
