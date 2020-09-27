# Julia wrapper for header: wlr_output_management_v1.h
# Automatically generated using Clang.jl


function wlr_output_manager_v1_create(display)
    ccall((:wlr_output_manager_v1_create, wlr_output_management_v1), Ptr{wlr_output_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_output_manager_v1_set_configuration(manager, config)
    ccall((:wlr_output_manager_v1_set_configuration, wlr_output_management_v1), Cvoid, (Ptr{wlr_output_manager_v1}, Ptr{wlr_output_configuration_v1}), manager, config)
end

function wlr_output_configuration_v1_create()
    ccall((:wlr_output_configuration_v1_create, wlr_output_management_v1), Ptr{wlr_output_configuration_v1}, ())
end

function wlr_output_configuration_v1_destroy(config)
    ccall((:wlr_output_configuration_v1_destroy, wlr_output_management_v1), Cvoid, (Ptr{wlr_output_configuration_v1},), config)
end

function wlr_output_configuration_v1_send_succeeded(config)
    ccall((:wlr_output_configuration_v1_send_succeeded, wlr_output_management_v1), Cvoid, (Ptr{wlr_output_configuration_v1},), config)
end

function wlr_output_configuration_v1_send_failed(config)
    ccall((:wlr_output_configuration_v1_send_failed, wlr_output_management_v1), Cvoid, (Ptr{wlr_output_configuration_v1},), config)
end

function wlr_output_configuration_head_v1_create(config, output)
    ccall((:wlr_output_configuration_head_v1_create, wlr_output_management_v1), Ptr{wlr_output_configuration_head_v1}, (Ptr{wlr_output_configuration_v1}, Ptr{wlr_output}), config, output)
end
