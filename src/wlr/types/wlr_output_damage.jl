# Julia wrapper for header: wlr_output_damage.h
# Automatically generated using Clang.jl


function wlr_output_damage_create(output)
    ccall((:wlr_output_damage_create, wlr_output_damage), Ptr{wlr_output_damage}, (Ptr{wlr_output},), output)
end

function wlr_output_damage_destroy(output_damage)
    ccall((:wlr_output_damage_destroy, wlr_output_damage), Cvoid, (Ptr{wlr_output_damage},), output_damage)
end

function wlr_output_damage_attach_render(output_damage, needs_frame, buffer_damage)
    ccall((:wlr_output_damage_attach_render, wlr_output_damage), Bool, (Ptr{wlr_output_damage}, Ptr{Bool}, Ptr{pixman_region32_t}), output_damage, needs_frame, buffer_damage)
end

function wlr_output_damage_add(output_damage, damage)
    ccall((:wlr_output_damage_add, wlr_output_damage), Cvoid, (Ptr{wlr_output_damage}, Ptr{pixman_region32_t}), output_damage, damage)
end

function wlr_output_damage_add_whole(output_damage)
    ccall((:wlr_output_damage_add_whole, wlr_output_damage), Cvoid, (Ptr{wlr_output_damage},), output_damage)
end

function wlr_output_damage_add_box(output_damage, box)
    ccall((:wlr_output_damage_add_box, wlr_output_damage), Cvoid, (Ptr{wlr_output_damage}, Ptr{wlr_box}), output_damage, box)
end
