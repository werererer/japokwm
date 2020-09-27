# Julia wrapper for header: wlr_gtk_primary_selection.h
# Automatically generated using Clang.jl


function wlr_gtk_primary_selection_device_manager_create(display)
    ccall((:wlr_gtk_primary_selection_device_manager_create, wlr_gtk_primary_selection), Ptr{wlr_gtk_primary_selection_device_manager}, (Ptr{wl_display},), display)
end
:Ptr{wlr_gtk_primary_selection_device_manager}
    seat::Ptr{wlr_seat}
    link::wl_list
    resources::wl_list
    offers::wl_list
    seat_destroy::wl_listener
    seat_focus_change::wl_listener
    seat_set_primary_selection::wl_listener
    data::Ptr{Cvoid}
end
