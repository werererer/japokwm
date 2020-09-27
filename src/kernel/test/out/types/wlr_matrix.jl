# Julia wrapper for header: wlr_matrix.h
# Automatically generated using Clang.jl


function wlr_matrix_identity(mat)
    ccall((:wlr_matrix_identity, wlr_matrix), Cvoid, (Ptr{Cfloat},), mat)
end

function wlr_matrix_multiply(mat, a, b)
    ccall((:wlr_matrix_multiply, wlr_matrix), Cvoid, (Ptr{Cfloat}, Ptr{Cfloat}, Ptr{Cfloat}), mat, a, b)
end

function wlr_matrix_transpose(mat, a)
    ccall((:wlr_matrix_transpose, wlr_matrix), Cvoid, (Ptr{Cfloat}, Ptr{Cfloat}), mat, a)
end

function wlr_matrix_translate(mat, x, y)
    ccall((:wlr_matrix_translate, wlr_matrix), Cvoid, (Ptr{Cfloat}, Cfloat, Cfloat), mat, x, y)
end

function wlr_matrix_scale(mat, x, y)
    ccall((:wlr_matrix_scale, wlr_matrix), Cvoid, (Ptr{Cfloat}, Cfloat, Cfloat), mat, x, y)
end

function wlr_matrix_rotate(mat, rad)
    ccall((:wlr_matrix_rotate, wlr_matrix), Cvoid, (Ptr{Cfloat}, Cfloat), mat, rad)
end

function wlr_matrix_transform(mat, transform)
    ccall((:wlr_matrix_transform, wlr_matrix), Cvoid, (Ptr{Cfloat}, wl_output_transform), mat, transform)
end

function wlr_matrix_projection(mat, width, height, transform)
    ccall((:wlr_matrix_projection, wlr_matrix), Cvoid, (Ptr{Cfloat}, Cint, Cint, wl_output_transform), mat, width, height, transform)
end

function wlr_matrix_project_box(mat, box, transform, rotation, projection)
    ccall((:wlr_matrix_project_box, wlr_matrix), Cvoid, (Ptr{Cfloat}, Ptr{wlr_box}, wl_output_transform, Cfloat, Ptr{Cfloat}), mat, box, transform, rotation, projection)
end
