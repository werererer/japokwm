# Julia wrapper for header: wlr_export_dmabuf_v1.h
# Automatically generated using Clang.jl


function wlr_export_dmabuf_manager_v1_create(display)
    ccall((:wlr_export_dmabuf_manager_v1_create, wlr_export_dmabuf_v1), Ptr{wlr_export_dmabuf_manager_v1}, (Ptr{wl_display},), display)
end
source::Ptr{wl_resource}
    manager::Ptr{wlr_export_dmabuf_manager_v1}
    link::wl_list
    attribs::wlr_dmabuf_attributes
    output::Ptr{wlr_output}
    cursor_locked::Bool
    output_precommit::wl_listener
end
