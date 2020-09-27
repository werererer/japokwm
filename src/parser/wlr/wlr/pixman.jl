# Julia wrapper for header: pixman-version.h
# Automatically generated using Clang.jl

# Julia wrapper for header: pixman.h
# Automatically generated using Clang.jl


function pixman_transform_init_identity(matrix)
    ccall((:pixman_transform_init_identity, pixman), Cvoid, (Ptr{pixman_transform},), matrix)
end

function pixman_transform_point_3d(transform, vector)
    ccall((:pixman_transform_point_3d, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_vector}), transform, vector)
end

function pixman_transform_point(transform, vector)
    ccall((:pixman_transform_point, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_vector}), transform, vector)
end

function pixman_transform_multiply(dst, l, r)
    ccall((:pixman_transform_multiply, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_transform}, Ptr{pixman_transform}), dst, l, r)
end

function pixman_transform_init_scale(t, sx, sy)
    ccall((:pixman_transform_init_scale, pixman), Cvoid, (Ptr{pixman_transform}, pixman_fixed_t, pixman_fixed_t), t, sx, sy)
end

function pixman_transform_scale(forward, reverse, sx, sy)
    ccall((:pixman_transform_scale, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_transform}, pixman_fixed_t, pixman_fixed_t), forward, reverse, sx, sy)
end

function pixman_transform_init_rotate(t, cos, sin)
    ccall((:pixman_transform_init_rotate, pixman), Cvoid, (Ptr{pixman_transform}, pixman_fixed_t, pixman_fixed_t), t, cos, sin)
end

function pixman_transform_rotate(forward, reverse, c, s)
    ccall((:pixman_transform_rotate, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_transform}, pixman_fixed_t, pixman_fixed_t), forward, reverse, c, s)
end

function pixman_transform_init_translate(t, tx, ty)
    ccall((:pixman_transform_init_translate, pixman), Cvoid, (Ptr{pixman_transform}, pixman_fixed_t, pixman_fixed_t), t, tx, ty)
end

function pixman_transform_translate(forward, reverse, tx, ty)
    ccall((:pixman_transform_translate, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_transform}, pixman_fixed_t, pixman_fixed_t), forward, reverse, tx, ty)
end

function pixman_transform_bounds(matrix, b)
    ccall((:pixman_transform_bounds, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_box16}), matrix, b)
end

function pixman_transform_invert(dst, src)
    ccall((:pixman_transform_invert, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_transform}), dst, src)
end

function pixman_transform_is_identity(t)
    ccall((:pixman_transform_is_identity, pixman), pixman_bool_t, (Ptr{pixman_transform},), t)
end

function pixman_transform_is_scale(t)
    ccall((:pixman_transform_is_scale, pixman), pixman_bool_t, (Ptr{pixman_transform},), t)
end

function pixman_transform_is_int_translate(t)
    ccall((:pixman_transform_is_int_translate, pixman), pixman_bool_t, (Ptr{pixman_transform},), t)
end

function pixman_transform_is_inverse(a, b)
    ccall((:pixman_transform_is_inverse, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_transform}), a, b)
end

function pixman_transform_from_pixman_f_transform(t, ft)
    ccall((:pixman_transform_from_pixman_f_transform, pixman), pixman_bool_t, (Ptr{pixman_transform}, Ptr{pixman_f_transform}), t, ft)
end

function pixman_f_transform_from_pixman_transform(ft, t)
    ccall((:pixman_f_transform_from_pixman_transform, pixman), Cvoid, (Ptr{pixman_f_transform}, Ptr{pixman_transform}), ft, t)
end

function pixman_f_transform_invert(dst, src)
    ccall((:pixman_f_transform_invert, pixman), pixman_bool_t, (Ptr{pixman_f_transform}, Ptr{pixman_f_transform}), dst, src)
end

function pixman_f_transform_point(t, v)
    ccall((:pixman_f_transform_point, pixman), pixman_bool_t, (Ptr{pixman_f_transform}, Ptr{pixman_f_vector}), t, v)
end

function pixman_f_transform_point_3d(t, v)
    ccall((:pixman_f_transform_point_3d, pixman), Cvoid, (Ptr{pixman_f_transform}, Ptr{pixman_f_vector}), t, v)
end

function pixman_f_transform_multiply(dst, l, r)
    ccall((:pixman_f_transform_multiply, pixman), Cvoid, (Ptr{pixman_f_transform}, Ptr{pixman_f_transform}, Ptr{pixman_f_transform}), dst, l, r)
end

function pixman_f_transform_init_scale(t, sx, sy)
    ccall((:pixman_f_transform_init_scale, pixman), Cvoid, (Ptr{pixman_f_transform}, Cdouble, Cdouble), t, sx, sy)
end

function pixman_f_transform_scale(forward, reverse, sx, sy)
    ccall((:pixman_f_transform_scale, pixman), pixman_bool_t, (Ptr{pixman_f_transform}, Ptr{pixman_f_transform}, Cdouble, Cdouble), forward, reverse, sx, sy)
end

function pixman_f_transform_init_rotate(t, cos, sin)
    ccall((:pixman_f_transform_init_rotate, pixman), Cvoid, (Ptr{pixman_f_transform}, Cdouble, Cdouble), t, cos, sin)
end

function pixman_f_transform_rotate(forward, reverse, c, s)
    ccall((:pixman_f_transform_rotate, pixman), pixman_bool_t, (Ptr{pixman_f_transform}, Ptr{pixman_f_transform}, Cdouble, Cdouble), forward, reverse, c, s)
end

function pixman_f_transform_init_translate(t, tx, ty)
    ccall((:pixman_f_transform_init_translate, pixman), Cvoid, (Ptr{pixman_f_transform}, Cdouble, Cdouble), t, tx, ty)
end

function pixman_f_transform_translate(forward, reverse, tx, ty)
    ccall((:pixman_f_transform_translate, pixman), pixman_bool_t, (Ptr{pixman_f_transform}, Ptr{pixman_f_transform}, Cdouble, Cdouble), forward, reverse, tx, ty)
end

function pixman_f_transform_bounds(t, b)
    ccall((:pixman_f_transform_bounds, pixman), pixman_bool_t, (Ptr{pixman_f_transform}, Ptr{pixman_box16}), t, b)
end

function pixman_f_transform_init_identity(t)
    ccall((:pixman_f_transform_init_identity, pixman), Cvoid, (Ptr{pixman_f_transform},), t)
end

function pixman_region_set_static_pointers(empty_box, empty_data, broken_data)
    ccall((:pixman_region_set_static_pointers, pixman), Cvoid, (Ptr{pixman_box16_t}, Ptr{pixman_region16_data_t}, Ptr{pixman_region16_data_t}), empty_box, empty_data, broken_data)
end

function pixman_region_init(region)
    ccall((:pixman_region_init, pixman), Cvoid, (Ptr{pixman_region16_t},), region)
end

function pixman_region_init_rect(region, x, y, width, height)
    ccall((:pixman_region_init_rect, pixman), Cvoid, (Ptr{pixman_region16_t}, Cint, Cint, UInt32, UInt32), region, x, y, width, height)
end

function pixman_region_init_rects(region, boxes, count)
    ccall((:pixman_region_init_rects, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_box16_t}, Cint), region, boxes, count)
end

function pixman_region_init_with_extents(region, extents)
    ccall((:pixman_region_init_with_extents, pixman), Cvoid, (Ptr{pixman_region16_t}, Ptr{pixman_box16_t}), region, extents)
end

function pixman_region_init_from_image(region, image)
    ccall((:pixman_region_init_from_image, pixman), Cvoid, (Ptr{pixman_region16_t}, Ptr{pixman_image_t}), region, image)
end

function pixman_region_fini(region)
    ccall((:pixman_region_fini, pixman), Cvoid, (Ptr{pixman_region16_t},), region)
end

function pixman_region_translate(region, x, y)
    ccall((:pixman_region_translate, pixman), Cvoid, (Ptr{pixman_region16_t}, Cint, Cint), region, x, y)
end

function pixman_region_copy(dest, source)
    ccall((:pixman_region_copy, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}), dest, source)
end

function pixman_region_intersect(new_reg, reg1, reg2)
    ccall((:pixman_region_intersect, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}, Ptr{pixman_region16_t}), new_reg, reg1, reg2)
end

function pixman_region_union(new_reg, reg1, reg2)
    ccall((:pixman_region_union, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}, Ptr{pixman_region16_t}), new_reg, reg1, reg2)
end

function pixman_region_union_rect(dest, source, x, y, width, height)
    ccall((:pixman_region_union_rect, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}, Cint, Cint, UInt32, UInt32), dest, source, x, y, width, height)
end

function pixman_region_intersect_rect(dest, source, x, y, width, height)
    ccall((:pixman_region_intersect_rect, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}, Cint, Cint, UInt32, UInt32), dest, source, x, y, width, height)
end

function pixman_region_subtract(reg_d, reg_m, reg_s)
    ccall((:pixman_region_subtract, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}, Ptr{pixman_region16_t}), reg_d, reg_m, reg_s)
end

function pixman_region_inverse(new_reg, reg1, inv_rect)
    ccall((:pixman_region_inverse, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}, Ptr{pixman_box16_t}), new_reg, reg1, inv_rect)
end

function pixman_region_contains_point(region, x, y, box)
    ccall((:pixman_region_contains_point, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Cint, Cint, Ptr{pixman_box16_t}), region, x, y, box)
end

function pixman_region_contains_rectangle(region, prect)
    ccall((:pixman_region_contains_rectangle, pixman), pixman_region_overlap_t, (Ptr{pixman_region16_t}, Ptr{pixman_box16_t}), region, prect)
end

function pixman_region_not_empty(region)
    ccall((:pixman_region_not_empty, pixman), pixman_bool_t, (Ptr{pixman_region16_t},), region)
end

function pixman_region_extents(region)
    ccall((:pixman_region_extents, pixman), Ptr{pixman_box16_t}, (Ptr{pixman_region16_t},), region)
end

function pixman_region_n_rects(region)
    ccall((:pixman_region_n_rects, pixman), Cint, (Ptr{pixman_region16_t},), region)
end

function pixman_region_rectangles(region, n_rects)
    ccall((:pixman_region_rectangles, pixman), Ptr{pixman_box16_t}, (Ptr{pixman_region16_t}, Ptr{Cint}), region, n_rects)
end

function pixman_region_equal(region1, region2)
    ccall((:pixman_region_equal, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_region16_t}), region1, region2)
end

function pixman_region_selfcheck(region)
    ccall((:pixman_region_selfcheck, pixman), pixman_bool_t, (Ptr{pixman_region16_t},), region)
end

function pixman_region_reset(region, box)
    ccall((:pixman_region_reset, pixman), Cvoid, (Ptr{pixman_region16_t}, Ptr{pixman_box16_t}), region, box)
end

function pixman_region_clear(region)
    ccall((:pixman_region_clear, pixman), Cvoid, (Ptr{pixman_region16_t},), region)
end

function pixman_region32_init(region)
    ccall((:pixman_region32_init, pixman), Cvoid, (Ptr{pixman_region32_t},), region)
end

function pixman_region32_init_rect(region, x, y, width, height)
    ccall((:pixman_region32_init_rect, pixman), Cvoid, (Ptr{pixman_region32_t}, Cint, Cint, UInt32, UInt32), region, x, y, width, height)
end

function pixman_region32_init_rects(region, boxes, count)
    ccall((:pixman_region32_init_rects, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_box32_t}, Cint), region, boxes, count)
end

function pixman_region32_init_with_extents(region, extents)
    ccall((:pixman_region32_init_with_extents, pixman), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_box32_t}), region, extents)
end

function pixman_region32_init_from_image(region, image)
    ccall((:pixman_region32_init_from_image, pixman), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_image_t}), region, image)
end

function pixman_region32_fini(region)
    ccall((:pixman_region32_fini, pixman), Cvoid, (Ptr{pixman_region32_t},), region)
end

function pixman_region32_translate(region, x, y)
    ccall((:pixman_region32_translate, pixman), Cvoid, (Ptr{pixman_region32_t}, Cint, Cint), region, x, y)
end

function pixman_region32_copy(dest, source)
    ccall((:pixman_region32_copy, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}), dest, source)
end

function pixman_region32_intersect(new_reg, reg1, reg2)
    ccall((:pixman_region32_intersect, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Ptr{pixman_region32_t}), new_reg, reg1, reg2)
end

function pixman_region32_union(new_reg, reg1, reg2)
    ccall((:pixman_region32_union, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Ptr{pixman_region32_t}), new_reg, reg1, reg2)
end

function pixman_region32_intersect_rect(dest, source, x, y, width, height)
    ccall((:pixman_region32_intersect_rect, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cint, Cint, UInt32, UInt32), dest, source, x, y, width, height)
end

function pixman_region32_union_rect(dest, source, x, y, width, height)
    ccall((:pixman_region32_union_rect, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cint, Cint, UInt32, UInt32), dest, source, x, y, width, height)
end

function pixman_region32_subtract(reg_d, reg_m, reg_s)
    ccall((:pixman_region32_subtract, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Ptr{pixman_region32_t}), reg_d, reg_m, reg_s)
end

function pixman_region32_inverse(new_reg, reg1, inv_rect)
    ccall((:pixman_region32_inverse, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Ptr{pixman_box32_t}), new_reg, reg1, inv_rect)
end

function pixman_region32_contains_point(region, x, y, box)
    ccall((:pixman_region32_contains_point, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Cint, Cint, Ptr{pixman_box32_t}), region, x, y, box)
end

function pixman_region32_contains_rectangle(region, prect)
    ccall((:pixman_region32_contains_rectangle, pixman), pixman_region_overlap_t, (Ptr{pixman_region32_t}, Ptr{pixman_box32_t}), region, prect)
end

function pixman_region32_not_empty(region)
    ccall((:pixman_region32_not_empty, pixman), pixman_bool_t, (Ptr{pixman_region32_t},), region)
end

function pixman_region32_extents(region)
    ccall((:pixman_region32_extents, pixman), Ptr{pixman_box32_t}, (Ptr{pixman_region32_t},), region)
end

function pixman_region32_n_rects(region)
    ccall((:pixman_region32_n_rects, pixman), Cint, (Ptr{pixman_region32_t},), region)
end

function pixman_region32_rectangles(region, n_rects)
    ccall((:pixman_region32_rectangles, pixman), Ptr{pixman_box32_t}, (Ptr{pixman_region32_t}, Ptr{Cint}), region, n_rects)
end

function pixman_region32_equal(region1, region2)
    ccall((:pixman_region32_equal, pixman), pixman_bool_t, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}), region1, region2)
end

function pixman_region32_selfcheck(region)
    ccall((:pixman_region32_selfcheck, pixman), pixman_bool_t, (Ptr{pixman_region32_t},), region)
end

function pixman_region32_reset(region, box)
    ccall((:pixman_region32_reset, pixman), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_box32_t}), region, box)
end

function pixman_region32_clear(region)
    ccall((:pixman_region32_clear, pixman), Cvoid, (Ptr{pixman_region32_t},), region)
end

function pixman_blt(src_bits, dst_bits, src_stride, dst_stride, src_bpp, dst_bpp, src_x, src_y, dest_x, dest_y, width, height)
    ccall((:pixman_blt, pixman), pixman_bool_t, (Ptr{UInt32}, Ptr{UInt32}, Cint, Cint, Cint, Cint, Cint, Cint, Cint, Cint, Cint, Cint), src_bits, dst_bits, src_stride, dst_stride, src_bpp, dst_bpp, src_x, src_y, dest_x, dest_y, width, height)
end

function pixman_fill(bits, stride, bpp, x, y, width, height, _xor)
    ccall((:pixman_fill, pixman), pixman_bool_t, (Ptr{UInt32}, Cint, Cint, Cint, Cint, Cint, Cint, UInt32), bits, stride, bpp, x, y, width, height, _xor)
end

function pixman_version()
    ccall((:pixman_version, pixman), Cint, ())
end

function pixman_version_string()
    ccall((:pixman_version_string, pixman), Cstring, ())
end

function pixman_format_supported_destination(format)
    ccall((:pixman_format_supported_destination, pixman), pixman_bool_t, (pixman_format_code_t,), format)
end

function pixman_format_supported_source(format)
    ccall((:pixman_format_supported_source, pixman), pixman_bool_t, (pixman_format_code_t,), format)
end

function pixman_image_create_solid_fill(color)
    ccall((:pixman_image_create_solid_fill, pixman), Ptr{pixman_image_t}, (Ptr{pixman_color_t},), color)
end

function pixman_image_create_linear_gradient(p1, p2, stops, n_stops)
    ccall((:pixman_image_create_linear_gradient, pixman), Ptr{pixman_image_t}, (Ptr{pixman_point_fixed_t}, Ptr{pixman_point_fixed_t}, Ptr{pixman_gradient_stop_t}, Cint), p1, p2, stops, n_stops)
end

function pixman_image_create_radial_gradient(inner, outer, inner_radius, outer_radius, stops, n_stops)
    ccall((:pixman_image_create_radial_gradient, pixman), Ptr{pixman_image_t}, (Ptr{pixman_point_fixed_t}, Ptr{pixman_point_fixed_t}, pixman_fixed_t, pixman_fixed_t, Ptr{pixman_gradient_stop_t}, Cint), inner, outer, inner_radius, outer_radius, stops, n_stops)
end

function pixman_image_create_conical_gradient(center, angle, stops, n_stops)
    ccall((:pixman_image_create_conical_gradient, pixman), Ptr{pixman_image_t}, (Ptr{pixman_point_fixed_t}, pixman_fixed_t, Ptr{pixman_gradient_stop_t}, Cint), center, angle, stops, n_stops)
end

function pixman_image_create_bits(format, width, height, bits, rowstride_bytes)
    ccall((:pixman_image_create_bits, pixman), Ptr{pixman_image_t}, (pixman_format_code_t, Cint, Cint, Ptr{UInt32}, Cint), format, width, height, bits, rowstride_bytes)
end

function pixman_image_create_bits_no_clear(format, width, height, bits, rowstride_bytes)
    ccall((:pixman_image_create_bits_no_clear, pixman), Ptr{pixman_image_t}, (pixman_format_code_t, Cint, Cint, Ptr{UInt32}, Cint), format, width, height, bits, rowstride_bytes)
end

function pixman_image_ref(image)
    ccall((:pixman_image_ref, pixman), Ptr{pixman_image_t}, (Ptr{pixman_image_t},), image)
end

function pixman_image_unref(image)
    ccall((:pixman_image_unref, pixman), pixman_bool_t, (Ptr{pixman_image_t},), image)
end

function pixman_image_set_destroy_function(image, _function, data)
    ccall((:pixman_image_set_destroy_function, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_image_destroy_func_t, Ptr{Cvoid}), image, _function, data)
end

function pixman_image_get_destroy_data(image)
    ccall((:pixman_image_get_destroy_data, pixman), Ptr{Cvoid}, (Ptr{pixman_image_t},), image)
end

function pixman_image_set_clip_region(image, region)
    ccall((:pixman_image_set_clip_region, pixman), pixman_bool_t, (Ptr{pixman_image_t}, Ptr{pixman_region16_t}), image, region)
end

function pixman_image_set_clip_region32(image, region)
    ccall((:pixman_image_set_clip_region32, pixman), pixman_bool_t, (Ptr{pixman_image_t}, Ptr{pixman_region32_t}), image, region)
end

function pixman_image_set_has_client_clip(image, clien_clip)
    ccall((:pixman_image_set_has_client_clip, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_bool_t), image, clien_clip)
end

function pixman_image_set_transform(image, transform)
    ccall((:pixman_image_set_transform, pixman), pixman_bool_t, (Ptr{pixman_image_t}, Ptr{pixman_transform_t}), image, transform)
end

function pixman_image_set_repeat(image, repeat)
    ccall((:pixman_image_set_repeat, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_repeat_t), image, repeat)
end

function pixman_image_set_dither(image, dither)
    ccall((:pixman_image_set_dither, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_dither_t), image, dither)
end

function pixman_image_set_dither_offset(image, offset_x, offset_y)
    ccall((:pixman_image_set_dither_offset, pixman), Cvoid, (Ptr{pixman_image_t}, Cint, Cint), image, offset_x, offset_y)
end

function pixman_image_set_filter(image, filter, filter_params, n_filter_params)
    ccall((:pixman_image_set_filter, pixman), pixman_bool_t, (Ptr{pixman_image_t}, pixman_filter_t, Ptr{pixman_fixed_t}, Cint), image, filter, filter_params, n_filter_params)
end

function pixman_image_set_source_clipping(image, source_clipping)
    ccall((:pixman_image_set_source_clipping, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_bool_t), image, source_clipping)
end

function pixman_image_set_alpha_map(image, alpha_map, x, y)
    ccall((:pixman_image_set_alpha_map, pixman), Cvoid, (Ptr{pixman_image_t}, Ptr{pixman_image_t}, Int16, Int16), image, alpha_map, x, y)
end

function pixman_image_set_component_alpha(image, component_alpha)
    ccall((:pixman_image_set_component_alpha, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_bool_t), image, component_alpha)
end

function pixman_image_get_component_alpha(image)
    ccall((:pixman_image_get_component_alpha, pixman), pixman_bool_t, (Ptr{pixman_image_t},), image)
end

function pixman_image_set_accessors(image, read_func, write_func)
    ccall((:pixman_image_set_accessors, pixman), Cvoid, (Ptr{pixman_image_t}, pixman_read_memory_func_t, pixman_write_memory_func_t), image, read_func, write_func)
end

function pixman_image_set_indexed(image, indexed)
    ccall((:pixman_image_set_indexed, pixman), Cvoid, (Ptr{pixman_image_t}, Ptr{pixman_indexed_t}), image, indexed)
end

function pixman_image_get_data(image)
    ccall((:pixman_image_get_data, pixman), Ptr{UInt32}, (Ptr{pixman_image_t},), image)
end

function pixman_image_get_width(image)
    ccall((:pixman_image_get_width, pixman), Cint, (Ptr{pixman_image_t},), image)
end

function pixman_image_get_height(image)
    ccall((:pixman_image_get_height, pixman), Cint, (Ptr{pixman_image_t},), image)
end

function pixman_image_get_stride(image)
    ccall((:pixman_image_get_stride, pixman), Cint, (Ptr{pixman_image_t},), image)
end

function pixman_image_get_depth(image)
    ccall((:pixman_image_get_depth, pixman), Cint, (Ptr{pixman_image_t},), image)
end

function pixman_image_get_format(image)
    ccall((:pixman_image_get_format, pixman), pixman_format_code_t, (Ptr{pixman_image_t},), image)
end

function pixman_filter_create_separable_convolution(n_values, scale_x, scale_y, reconstruct_x, reconstruct_y, sample_x, sample_y, subsample_bits_x, subsample_bits_y)
    ccall((:pixman_filter_create_separable_convolution, pixman), Ptr{pixman_fixed_t}, (Ptr{Cint}, pixman_fixed_t, pixman_fixed_t, pixman_kernel_t, pixman_kernel_t, pixman_kernel_t, pixman_kernel_t, Cint, Cint), n_values, scale_x, scale_y, reconstruct_x, reconstruct_y, sample_x, sample_y, subsample_bits_x, subsample_bits_y)
end

function pixman_image_fill_rectangles(op, image, color, n_rects, rects)
    ccall((:pixman_image_fill_rectangles, pixman), pixman_bool_t, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_color_t}, Cint, Ptr{pixman_rectangle16_t}), op, image, color, n_rects, rects)
end

function pixman_image_fill_boxes(op, dest, color, n_boxes, boxes)
    ccall((:pixman_image_fill_boxes, pixman), pixman_bool_t, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_color_t}, Cint, Ptr{pixman_box32_t}), op, dest, color, n_boxes, boxes)
end

function pixman_compute_composite_region(region, src_image, mask_image, dest_image, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height)
    ccall((:pixman_compute_composite_region, pixman), pixman_bool_t, (Ptr{pixman_region16_t}, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Int16, Int16, Int16, Int16, Int16, Int16, UInt16, UInt16), region, src_image, mask_image, dest_image, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height)
end

function pixman_image_composite(op, src, mask, dest, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height)
    ccall((:pixman_image_composite, pixman), Cvoid, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Int16, Int16, Int16, Int16, Int16, Int16, UInt16, UInt16), op, src, mask, dest, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height)
end

function pixman_image_composite32(op, src, mask, dest, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height)
    ccall((:pixman_image_composite32, pixman), Cvoid, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Int32, Int32, Int32, Int32, Int32, Int32, Int32, Int32), op, src, mask, dest, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height)
end

function pixman_disable_out_of_bounds_workaround()
    ccall((:pixman_disable_out_of_bounds_workaround, pixman), Cvoid, ())
end

function pixman_glyph_cache_create()
    ccall((:pixman_glyph_cache_create, pixman), Ptr{pixman_glyph_cache_t}, ())
end

function pixman_glyph_cache_destroy(cache)
    ccall((:pixman_glyph_cache_destroy, pixman), Cvoid, (Ptr{pixman_glyph_cache_t},), cache)
end

function pixman_glyph_cache_freeze(cache)
    ccall((:pixman_glyph_cache_freeze, pixman), Cvoid, (Ptr{pixman_glyph_cache_t},), cache)
end

function pixman_glyph_cache_thaw(cache)
    ccall((:pixman_glyph_cache_thaw, pixman), Cvoid, (Ptr{pixman_glyph_cache_t},), cache)
end

function pixman_glyph_cache_lookup(cache, font_key, glyph_key)
    ccall((:pixman_glyph_cache_lookup, pixman), Ptr{Cvoid}, (Ptr{pixman_glyph_cache_t}, Ptr{Cvoid}, Ptr{Cvoid}), cache, font_key, glyph_key)
end

function pixman_glyph_cache_insert(cache, font_key, glyph_key, origin_x, origin_y, glyph_image)
    ccall((:pixman_glyph_cache_insert, pixman), Ptr{Cvoid}, (Ptr{pixman_glyph_cache_t}, Ptr{Cvoid}, Ptr{Cvoid}, Cint, Cint, Ptr{pixman_image_t}), cache, font_key, glyph_key, origin_x, origin_y, glyph_image)
end

function pixman_glyph_cache_remove(cache, font_key, glyph_key)
    ccall((:pixman_glyph_cache_remove, pixman), Cvoid, (Ptr{pixman_glyph_cache_t}, Ptr{Cvoid}, Ptr{Cvoid}), cache, font_key, glyph_key)
end

function pixman_glyph_get_extents(cache, n_glyphs, glyphs, extents)
    ccall((:pixman_glyph_get_extents, pixman), Cvoid, (Ptr{pixman_glyph_cache_t}, Cint, Ptr{pixman_glyph_t}, Ptr{pixman_box32_t}), cache, n_glyphs, glyphs, extents)
end

function pixman_glyph_get_mask_format(cache, n_glyphs, glyphs)
    ccall((:pixman_glyph_get_mask_format, pixman), pixman_format_code_t, (Ptr{pixman_glyph_cache_t}, Cint, Ptr{pixman_glyph_t}), cache, n_glyphs, glyphs)
end

function pixman_composite_glyphs(op, src, dest, mask_format, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height, cache, n_glyphs, glyphs)
    ccall((:pixman_composite_glyphs, pixman), Cvoid, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_image_t}, pixman_format_code_t, Int32, Int32, Int32, Int32, Int32, Int32, Int32, Int32, Ptr{pixman_glyph_cache_t}, Cint, Ptr{pixman_glyph_t}), op, src, dest, mask_format, src_x, src_y, mask_x, mask_y, dest_x, dest_y, width, height, cache, n_glyphs, glyphs)
end

function pixman_composite_glyphs_no_mask(op, src, dest, src_x, src_y, dest_x, dest_y, cache, n_glyphs, glyphs)
    ccall((:pixman_composite_glyphs_no_mask, pixman), Cvoid, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_image_t}, Int32, Int32, Int32, Int32, Ptr{pixman_glyph_cache_t}, Cint, Ptr{pixman_glyph_t}), op, src, dest, src_x, src_y, dest_x, dest_y, cache, n_glyphs, glyphs)
end

function pixman_sample_ceil_y(y, bpp)
    ccall((:pixman_sample_ceil_y, pixman), pixman_fixed_t, (pixman_fixed_t, Cint), y, bpp)
end

function pixman_sample_floor_y(y, bpp)
    ccall((:pixman_sample_floor_y, pixman), pixman_fixed_t, (pixman_fixed_t, Cint), y, bpp)
end

function pixman_edge_step(e, n)
    ccall((:pixman_edge_step, pixman), Cvoid, (Ptr{pixman_edge_t}, Cint), e, n)
end

function pixman_edge_init(e, bpp, y_start, x_top, y_top, x_bot, y_bot)
    ccall((:pixman_edge_init, pixman), Cvoid, (Ptr{pixman_edge_t}, Cint, pixman_fixed_t, pixman_fixed_t, pixman_fixed_t, pixman_fixed_t, pixman_fixed_t), e, bpp, y_start, x_top, y_top, x_bot, y_bot)
end

function pixman_line_fixed_edge_init(e, bpp, y, line, x_off, y_off)
    ccall((:pixman_line_fixed_edge_init, pixman), Cvoid, (Ptr{pixman_edge_t}, Cint, pixman_fixed_t, Ptr{pixman_line_fixed_t}, Cint, Cint), e, bpp, y, line, x_off, y_off)
end

function pixman_rasterize_edges(image, l, r, t, b)
    ccall((:pixman_rasterize_edges, pixman), Cvoid, (Ptr{pixman_image_t}, Ptr{pixman_edge_t}, Ptr{pixman_edge_t}, pixman_fixed_t, pixman_fixed_t), image, l, r, t, b)
end

function pixman_add_traps(image, x_off, y_off, ntrap, traps)
    ccall((:pixman_add_traps, pixman), Cvoid, (Ptr{pixman_image_t}, Int16, Int16, Cint, Ptr{pixman_trap_t}), image, x_off, y_off, ntrap, traps)
end

function pixman_add_trapezoids(image, x_off, y_off, ntraps, traps)
    ccall((:pixman_add_trapezoids, pixman), Cvoid, (Ptr{pixman_image_t}, Int16, Cint, Cint, Ptr{pixman_trapezoid_t}), image, x_off, y_off, ntraps, traps)
end

function pixman_rasterize_trapezoid(image, trap, x_off, y_off)
    ccall((:pixman_rasterize_trapezoid, pixman), Cvoid, (Ptr{pixman_image_t}, Ptr{pixman_trapezoid_t}, Cint, Cint), image, trap, x_off, y_off)
end

function pixman_composite_trapezoids(op, src, dst, mask_format, x_src, y_src, x_dst, y_dst, n_traps, traps)
    ccall((:pixman_composite_trapezoids, pixman), Cvoid, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_image_t}, pixman_format_code_t, Cint, Cint, Cint, Cint, Cint, Ptr{pixman_trapezoid_t}), op, src, dst, mask_format, x_src, y_src, x_dst, y_dst, n_traps, traps)
end

function pixman_composite_triangles(op, src, dst, mask_format, x_src, y_src, x_dst, y_dst, n_tris, tris)
    ccall((:pixman_composite_triangles, pixman), Cvoid, (pixman_op_t, Ptr{pixman_image_t}, Ptr{pixman_image_t}, pixman_format_code_t, Cint, Cint, Cint, Cint, Cint, Ptr{pixman_triangle_t}), op, src, dst, mask_format, x_src, y_src, x_dst, y_dst, n_tris, tris)
end

function pixman_add_triangles(image, x_off, y_off, n_tris, tris)
    ccall((:pixman_add_triangles, pixman), Cvoid, (Ptr{pixman_image_t}, Int32, Int32, Cint, Ptr{pixman_triangle_t}), image, x_off, y_off, n_tris, tris)
end
