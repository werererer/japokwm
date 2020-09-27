# Julia wrapper for header: wlr_xdg_output_v1.h
# Automatically generated using Clang.jl


function wlr_xdg_output_manager_v1_create(display, layout)
    ccall((:wlr_xdg_output_manager_v1_create, wlr_xdg_output_v1), Ptr{wlr_xdg_output_manager_v1}, (Ptr{wl_display}, Ptr{wlr_output_layout}), display, layout)
end
ut_change::wl_listener
    layout_destroy::wl_listener
end

struct wlr_xdg_output_v1
    manager::Ptr{wlr_xdg_output_manager_v1}
    resources::wl_list
    link::wl_list
    layout_output::Ptr{wlr_output_layout_output}
    x::Int32
    y::Int32
    width::Int32
    height::Int32
    destroy::wl_listener
    description::wl_listener
end
