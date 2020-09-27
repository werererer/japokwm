# Julia wrapper for header: region.h
# Automatically generated using Clang.jl


function wlr_region_scale(dst, src, scale)
    ccall((:wlr_region_scale, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cfloat), dst, src, scale)
end

function wlr_region_scale_xy(dst, src, scale_x, scale_y)
    ccall((:wlr_region_scale_xy, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cfloat, Cfloat), dst, src, scale_x, scale_y)
end

function wlr_region_transform(dst, src, transform, width, height)
    ccall((:wlr_region_transform, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, wl_output_transform, Cint, Cint), dst, src, transform, width, height)
end

function wlr_region_expand(dst, src, distance)
    ccall((:wlr_region_expand, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cint), dst, src, distance)
end

function wlr_region_rotated_bounds(dst, src, rotation, ox, oy)
    ccall((:wlr_region_rotated_bounds, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cfloat, Cint, Cint), dst, src, rotation, ox, oy)
end

function wlr_region_confine(region, x1, y1, x2, y2, x2_out, y2_out)
    ccall((:wlr_region_confine, region), Bool, (Ptr{pixman_region32_t}, Cdouble, Cdouble, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), region, x1, y1, x2, y2, x2_out, y2_out)
end
