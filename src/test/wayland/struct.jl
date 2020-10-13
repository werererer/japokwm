struct wl_list
    prev::Ptr{wl_list}
    next::Ptr{wl_list}
end

struct wl_listener
    link::wl_list
    notify::wl_notify_func_t
end

struct wl_interface
    name::Cstring
    version::Cint
    method_count::Cint
    methods::Ptr{wl_message}
    event_count::Cint
    events::Ptr{wl_message}
end

struct wl_signal
    listener_list::wl_list
end

struct wl_message
    name::Cstring
    signature::Cstring
    types::Ptr{Ptr{wl_interface}}
end

struct wl_argument
    s::Cstring
end

struct wl_display_listener
    error::Ptr{Cvoid}
    delete_id::Ptr{Cvoid}
end

struct wl_callback_listener
    done::Ptr{Cvoid}
end

struct wl_shm_listener
    format::Ptr{Cvoid}
end

struct wl_data_offer_listener
    offer::Ptr{Cvoid}
    source_actions::Ptr{Cvoid}
    action::Ptr{Cvoid}
end

struct wl_data_source_listener
    target::Ptr{Cvoid}
    send::Ptr{Cvoid}
    cancelled::Ptr{Cvoid}
    dnd_drop_performed::Ptr{Cvoid}
    dnd_finished::Ptr{Cvoid}
    action::Ptr{Cvoid}
end

struct wl_data_device_listener
    data_offer::Ptr{Cvoid}
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
    motion::Ptr{Cvoid}
    drop::Ptr{Cvoid}
    selection::Ptr{Cvoid}
end

struct wl_shell_surface_listener
    ping::Ptr{Cvoid}
    configure::Ptr{Cvoid}
    popup_done::Ptr{Cvoid}
end

struct wl_surface_listener
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
end

struct wl_seat_listener
    capabilities::Ptr{Cvoid}
    name::Ptr{Cvoid}
end

struct wl_pointer_listener
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
    motion::Ptr{Cvoid}
    button::Ptr{Cvoid}
    axis::Ptr{Cvoid}
    frame::Ptr{Cvoid}
    axis_source::Ptr{Cvoid}
    axis_stop::Ptr{Cvoid}
    axis_discrete::Ptr{Cvoid}
end

struct wl_keyboard_listener
    keymap::Ptr{Cvoid}
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
    key::Ptr{Cvoid}
    modifiers::Ptr{Cvoid}
    repeat_info::Ptr{Cvoid}
end

struct wl_output_listener
    geometry::Ptr{Cvoid}
    mode::Ptr{Cvoid}
    done::Ptr{Cvoid}
    scale::Ptr{Cvoid}
end

struct wl_cursor_image
    width::UInt32
    height::UInt32
    hotspot_x::UInt32
    hotspot_y::UInt32
    delay::UInt32
end

struct wl_egl_window
    version::intptr_t
    width::Cint
    height::Cint
    dx::Cint
    dy::Cint
    attached_width::Cint
    attached_height::Cint
    driver_private::Ptr{Cvoid}
    resize_callback::Ptr{Cvoid}
    destroy_window_callback::Ptr{Cvoid}
    surface::Ptr{wl_surface}
end

struct wl_display_interface
    sync::Ptr{Cvoid}
    get_registry::Ptr{Cvoid}
end

struct wl_compositor_interface
    create_surface::Ptr{Cvoid}
    create_region::Ptr{Cvoid}
end

struct wl_shm_interface
    create_pool::Ptr{Cvoid}
end

struct wl_data_offer_interface
    accept::Ptr{Cvoid}
    receive::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    finish::Ptr{Cvoid}
    set_actions::Ptr{Cvoid}
end

struct wl_data_device_interface
    start_drag::Ptr{Cvoid}
    set_selection::Ptr{Cvoid}
    release::Ptr{Cvoid}
end

struct wl_shell_interface
    get_shell_surface::Ptr{Cvoid}
end

struct wl_surface_interface
    destroy::Ptr{Cvoid}
    attach::Ptr{Cvoid}
    damage::Ptr{Cvoid}
    frame::Ptr{Cvoid}
    set_opaque_region::Ptr{Cvoid}
    set_input_region::Ptr{Cvoid}
    commit::Ptr{Cvoid}
    set_buffer_transform::Ptr{Cvoid}
    set_buffer_scale::Ptr{Cvoid}
    damage_buffer::Ptr{Cvoid}
end

struct wl_pointer_interface
    set_cursor::Ptr{Cvoid}
    release::Ptr{Cvoid}
end

struct wl_touch_interface
    release::Ptr{Cvoid}
end

struct wl_region_interface
    destroy::Ptr{Cvoid}
    add::Ptr{Cvoid}
    subtract::Ptr{Cvoid}
end

struct wl_subsurface_interface
    destroy::Ptr{Cvoid}
    set_position::Ptr{Cvoid}
    place_above::Ptr{Cvoid}
    place_below::Ptr{Cvoid}
    set_sync::Ptr{Cvoid}
    set_desync::Ptr{Cvoid}
end

struct wl_array
    size::Csize_t
    alloc::Csize_t
    data::Ptr{Cvoid}
end

struct wl_object
    interface::Ptr{wl_interface}
    implementation::Ptr{Cvoid}
    id::UInt32
end

struct wl_resource
    object::wl_object
    destroy::wl_resource_destroy_func_t
    link::wl_list
    destroy_signal::wl_signal
    client::Ptr{wl_client}
    data::Ptr{Cvoid}
end

struct wl_protocol_logger_message
    resource::Ptr{wl_resource}
    message_opcode::Cint
    message::Ptr{wl_message}
    arguments_count::Cint
    arguments::Ptr{wl_argument}
end

struct wl_registry_listener
    _global::Ptr{Cvoid}
    global_remove::Ptr{Cvoid}
end

struct wl_buffer_listener
    release::Ptr{Cvoid}
end

struct wl_touch_listener
    down::Ptr{Cvoid}
    up::Ptr{Cvoid}
    motion::Ptr{Cvoid}
    frame::Ptr{Cvoid}
    cancel::Ptr{Cvoid}
    shape::Ptr{Cvoid}
    orientation::Ptr{Cvoid}
end

struct wl_cursor
    image_count::UInt32
    images::Ptr{Ptr{wl_cursor_image}}
    name::Cstring
end

struct wl_registry_interface
    bind::Ptr{Cvoid}
end

struct wl_buffer_interface
    destroy::Ptr{Cvoid}
end

struct wl_data_device_manager_interface
    create_data_source::Ptr{Cvoid}
    get_data_device::Ptr{Cvoid}
end

struct wl_seat_interface
    get_pointer::Ptr{Cvoid}
    get_keyboard::Ptr{Cvoid}
    get_touch::Ptr{Cvoid}
    release::Ptr{Cvoid}
end

struct wl_output_interface
    release::Ptr{Cvoid}
end

struct wl_shm_pool_interface
    create_buffer::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    resize::Ptr{Cvoid}
end

struct wl_shell_surface_interface
    pong::Ptr{Cvoid}
    move::Ptr{Cvoid}
    resize::Ptr{Cvoid}
    set_toplevel::Ptr{Cvoid}
    set_transient::Ptr{Cvoid}
    set_fullscreen::Ptr{Cvoid}
    set_popup::Ptr{Cvoid}
    set_maximized::Ptr{Cvoid}
    set_title::Ptr{Cvoid}
    set_class::Ptr{Cvoid}
end

struct wl_subcompositor_interface
    destroy::Ptr{Cvoid}
    get_subsurface::Ptr{Cvoid}
end

struct wl_data_source_interface
    offer::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    set_actions::Ptr{Cvoid}
end

struct wl_keyboard_interface
    release::Ptr{Cvoid}
end
