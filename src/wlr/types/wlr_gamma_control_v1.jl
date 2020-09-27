# Julia wrapper for header: wlr_gamma_control_v1.h
# Automatically generated using Clang.jl


function wlr_gamma_control_manager_v1_create(display)
    ccall((:wlr_gamma_control_manager_v1_create, wlr_gamma_control_v1), Ptr{wlr_gamma_control_manager_v1}, (Ptr{wl_display},), display)
end
control_v1
    resource::Ptr{wl_resource}
    output::Ptr{wlr_output}
    link::wl_list
    output_destroy_listener::wl_listener
    data::Ptr{Cvoid}
end
