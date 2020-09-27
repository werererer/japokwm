# Julia wrapper for header: wlr_screencopy_v1.h
# Automatically generated using Clang.jl


function wlr_screencopy_manager_v1_create(display)
    ccall((:wlr_screencopy_manager_v1_create, wlr_screencopy_v1), Ptr{wlr_screencopy_manager_v1}, (Ptr{wl_display},), display)
end
wlr_screencopy_v1_client
    ref::Cint
    manager::Ptr{wlr_screencopy_manager_v1}
    damages::wl_list
end

struct wlr_screencopy_frame_v1
    resource::Ptr{wl_resource}
    client::Ptr{wlr_screencopy_v1_client}
    link::wl_list
    format::wl_shm_format
    fourcc::UInt32
    box::wlr_box
    stride::Cint
    overlay_cursor::Bool
    cursor_locked::Bool
    with_damage::Bool
    shm_buffer::Ptr{wl_shm_buffer}
    dma_buffer::Ptr{wlr_dmabuf_v1_buffer}
    buffer_destroy::wl_listener
    output::Ptr{wlr_output}
    output_precommit::wl_listener
    output_destroy::wl_listener
    output_enable::wl_listener
    data::Ptr{Cvoid}
end
