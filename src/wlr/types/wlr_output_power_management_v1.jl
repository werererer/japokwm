# Julia wrapper for header: wlr_output_power_management_v1.h
# Automatically generated using Clang.jl


function wlr_output_power_manager_v1_create(display)
    ccall((:wlr_output_power_manager_v1_create, wlr_output_power_management_v1), Ptr{wlr_output_power_manager_v1}, (Ptr{wl_display},), display)
end
 wlr_output_power_v1
    resource::Ptr{wl_resource}
    output::Ptr{wlr_output}
    manager::Ptr{wlr_output_power_manager_v1}
    link::wl_list
    output_destroy_listener::wl_listener
    output_enable_listener::wl_listener
    data::Ptr{Cvoid}
end
