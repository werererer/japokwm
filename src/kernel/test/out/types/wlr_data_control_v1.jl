# Julia wrapper for header: wlr_data_control_v1.h
# Automatically generated using Clang.jl


function wlr_data_control_manager_v1_create(display)
    ccall((:wlr_data_control_manager_v1_create, wlr_data_control_v1), Ptr{wlr_data_control_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_data_control_device_v1_destroy(device)
    ccall((:wlr_data_control_device_v1_destroy, wlr_data_control_v1), Cvoid, (Ptr{wlr_data_control_device_v1},), device)
end
wl_resource}
    primary_selection_offer_resource::Ptr{wl_resource}
    seat_destroy::wl_listener
    seat_set_selection::wl_listener
    seat_set_primary_selection::wl_listener
end
