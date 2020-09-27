# Automatically generated using Clang.jl

include("./wayland.jl")
include("./pixman.jl")

struct wlr_backend_impl
    start::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    get_renderer::Ptr{Cvoid}
    get_session::Ptr{Cvoid}
    get_presentation_clock::Ptr{Cvoid}
end

struct ANONYMOUS1_events
    destroy::wl_signal
    new_input::wl_signal
    new_output::wl_signal
end

struct wlr_backend
    impl::Ptr{wlr_backend_impl}
    events::ANONYMOUS1_events
end

struct wlr_renderer_impl
    _begin::Ptr{Cvoid}
    _end::Ptr{Cvoid}
    clear::Ptr{Cvoid}
    scissor::Ptr{Cvoid}
    render_subtexture_with_matrix::Ptr{Cvoid}
    render_quad_with_matrix::Ptr{Cvoid}
    render_ellipse_with_matrix::Ptr{Cvoid}
    formats::Ptr{Cvoid}
    format_supported::Ptr{Cvoid}
    resource_is_wl_drm_buffer::Ptr{Cvoid}
    wl_drm_buffer_get_size::Ptr{Cvoid}
    get_dmabuf_formats::Ptr{Cvoid}
    preferred_read_format::Ptr{Cvoid}
    read_pixels::Ptr{Cvoid}
    texture_from_pixels::Ptr{Cvoid}
    texture_from_wl_drm::Ptr{Cvoid}
    texture_from_dmabuf::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    init_wl_display::Ptr{Cvoid}
    blit_dmabuf::Ptr{Cvoid}
end

struct ANONYMOUS100_events
    destroy::wl_signal
end

struct wlr_renderer
    impl::Ptr{wlr_renderer_impl}
    rendering::Bool
    events::ANONYMOUS100_events
end

const wlr_renderer_create_func_t = Ptr{Cvoid}
const _drmModeModeInfo = Cvoid
const drmModeModeInfo = _drmModeModeInfo

struct session_impl
    create::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    open::Ptr{Cvoid}
    close::Ptr{Cvoid}
    change_vt::Ptr{Cvoid}
end

struct wlr_device
    fd::Cint
    dev::Cint
    signal::wl_signal
    link::wl_list
end

struct ANONYMOUS2_events
    destroy::wl_signal
end

struct wlr_session
    impl::Ptr{session_impl}
    session_signal::wl_signal
    active::Bool
    vtnr::UInt32
    seat::NTuple{256, UInt8}
    udev::Ptr{Cint}
    mon::Ptr{Cint}
    udev_event::Ptr{wl_event_source}
    devices::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS2_events
end

struct wlr_input_device_impl
    destroy::Ptr{Cvoid}
end

struct wlr_keyboard_impl
    destroy::Ptr{Cvoid}
    led_update::Ptr{Cvoid}
end

struct wlr_touch_impl
    destroy::Ptr{Cvoid}
end

struct wlr_switch_impl
    destroy::Ptr{Cvoid}
end

struct wlr_tablet_impl
    destroy::Ptr{Cvoid}
end

struct wlr_pointer_impl
    destroy::Ptr{Cvoid}
end

struct wlr_tablet_pad_impl
    destroy::Ptr{Cvoid}
end

struct wlr_output_impl
    set_cursor::Ptr{Cvoid}
    move_cursor::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    attach_render::Ptr{Cvoid}
    rollback_render::Ptr{Cvoid}
    test::Ptr{Cvoid}
    commit::Ptr{Cvoid}
    get_gamma_size::Ptr{Cvoid}
    export_dmabuf::Ptr{Cvoid}
end

struct ANONYMOUS3_events
    destroy::wl_signal
end

struct wlr_data_device_manager
    _global::Ptr{wl_global}
    data_sources::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS3_events
    data::Ptr{Cvoid}
end

@cenum wlr_data_offer_type::UInt32 begin
    WLR_DATA_OFFER_SELECTION = 0
    WLR_DATA_OFFER_DRAG = 1
end


struct wlr_data_source_impl
    send::Ptr{Cvoid}
    accept::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    dnd_drop::Ptr{Cvoid}
    dnd_finish::Ptr{Cvoid}
    dnd_action::Ptr{Cvoid}
end

struct ANONYMOUS4_events
    destroy::wl_signal
end

struct wlr_data_source
    impl::Ptr{wlr_data_source_impl}
    mime_types::wl_array
    actions::Int32
    accepted::Bool
    current_dnd_action::wl_data_device_manager_dnd_action
    compositor_action::UInt32
    events::ANONYMOUS4_events
end

struct wlr_data_offer
    resource::Ptr{wl_resource}
    source::Ptr{wlr_data_source}
    type::wlr_data_offer_type
    link::wl_list
    actions::UInt32
    preferred_action::wl_data_device_manager_dnd_action
    in_ask::Bool
    source_destroy::wl_listener
end

struct ANONYMOUS5_events
    focus::wl_signal
    motion::wl_signal
    drop::wl_signal
    destroy::wl_signal
end

@cenum wlr_drag_grab_type::UInt32 begin
    WLR_DRAG_GRAB_KEYBOARD = 0
    WLR_DRAG_GRAB_KEYBOARD_POINTER = 1
    WLR_DRAG_GRAB_KEYBOARD_TOUCH = 2
end


struct wlr_keyboard_grab_interface
    enter::Ptr{Cvoid}
    clear_focus::Ptr{Cvoid}
    key::Ptr{Cvoid}
    modifiers::Ptr{Cvoid}
    cancel::Ptr{Cvoid}
end

struct wlr_primary_selection_source_impl
    send::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
end

struct ANONYMOUS13_events
    destroy::wl_signal
end

struct wlr_primary_selection_source
    impl::Ptr{wlr_primary_selection_source_impl}
    mime_types::wl_array
    events::ANONYMOUS13_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS62_events
    destroy::wl_signal
end

struct wlr_serial_range
    min_incl::UInt32
    max_incl::UInt32
end

struct wlr_serial_ringset
    data::NTuple{128, wlr_serial_range}
    _end::Cint
    count::Cint
end

abstract type Abstract_wlr_seat end
struct wlr_seat_client{wlr_seat<:Abstract_wlr_seat}
    client::Ptr{wl_client}
    seat::Ptr{wlr_seat}
    link::wl_list
    resources::wl_list
    pointers::wl_list
    keyboards::wl_list
    touches::wl_list
    data_devices::wl_list
    events::ANONYMOUS62_events
    serials::wlr_serial_ringset
end

struct wlr_buffer_impl
    destroy::Ptr{Cvoid}
    get_dmabuf::Ptr{Cvoid}
end

struct ANONYMOUS30_events
    destroy::wl_signal
    release::wl_signal
end

struct wlr_buffer
    impl::Ptr{wlr_buffer_impl}
    width::Cint
    height::Cint
    dropped::Bool
    n_locks::Csize_t
    events::ANONYMOUS30_events
end

struct wlr_texture_impl
    is_opaque::Ptr{Cvoid}
    write_pixels::Ptr{Cvoid}
    to_dmabuf::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
end

struct wlr_texture
    impl::Ptr{wlr_texture_impl}
    width::UInt32
    height::UInt32
end

struct wlr_client_buffer
    base::wlr_buffer
    resource::Ptr{wl_resource}
    resource_released::Bool
    texture::Ptr{wlr_texture}
    resource_destroy::wl_listener
    release::wl_listener
end

struct wlr_fbox
    x::Cdouble
    y::Cdouble
    width::Cdouble
    height::Cdouble
end

struct ANONYMOUS27_viewport
    has_src::Bool
    has_dst::Bool
    src::wlr_fbox
    dst_width::Cint
    dst_height::Cint
end

struct wlr_surface_state
    committed::UInt32
    buffer_resource::Ptr{wl_resource}
    dx::Int32
    dy::Int32
    surface_damage::pixman_region32_t
    buffer_damage::pixman_region32_t
    opaque::pixman_region32_t
    input::pixman_region32_t
    transform::wl_output_transform
    scale::Int32
    frame_callback_list::wl_list
    width::Cint
    height::Cint
    buffer_width::Cint
    buffer_height::Cint
    viewport::ANONYMOUS27_viewport
    buffer_destroy::wl_listener
end

struct wlr_surface_role
    name::Cstring
    commit::Ptr{Cvoid}
    precommit::Ptr{Cvoid}
end

struct ANONYMOUS28_events
    commit::wl_signal
    new_subsurface::wl_signal
    destroy::wl_signal
end

struct wlr_surface
    resource::Ptr{wl_resource}
    renderer::Ptr{wlr_renderer}
    buffer::Ptr{wlr_client_buffer}
    sx::Cint
    sy::Cint
    buffer_damage::pixman_region32_t
    opaque_region::pixman_region32_t
    input_region::pixman_region32_t
    current::wlr_surface_state
    pending::wlr_surface_state
    previous::wlr_surface_state
    role::Ptr{wlr_surface_role}
    role_data::Ptr{Cvoid}
    events::ANONYMOUS28_events
    subsurfaces::wl_list
    subsurface_pending_list::wl_list
    renderer_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_pointer_grab_interface
    enter::Ptr{Cvoid}
    clear_focus::Ptr{Cvoid}
    motion::Ptr{Cvoid}
    button::Ptr{Cvoid}
    axis::Ptr{Cvoid}
    frame::Ptr{Cvoid}
    cancel::Ptr{Cvoid}
end

struct wlr_seat_pointer_grab{wlr_seat<:Abstract_wlr_seat}

    interface::Ptr{wlr_pointer_grab_interface}
    seat::Ptr{wlr_seat}
    data::Ptr{Cvoid}
end

struct ANONYMOUS64_events
    focus_change::wl_signal
end

struct wlr_seat_pointer_state{wlr_seat<:Abstract_wlr_seat}

    seat::Ptr{wlr_seat}
    focused_client::Ptr{wlr_seat_client}
    focused_surface::Ptr{wlr_surface}
    sx::Cdouble
    sy::Cdouble
    grab::Ptr{wlr_seat_pointer_grab}
    default_grab::Ptr{wlr_seat_pointer_grab}
    buttons::NTuple{16, UInt32}
    button_count::Csize_t
    grab_button::UInt32
    grab_serial::UInt32
    grab_time::UInt32
    surface_destroy::wl_listener
    events::ANONYMOUS64_events
end

@cenum wlr_input_device_type::UInt32 begin
    WLR_INPUT_DEVICE_KEYBOARD = 0
    WLR_INPUT_DEVICE_POINTER = 1
    WLR_INPUT_DEVICE_TOUCH = 2
    WLR_INPUT_DEVICE_TABLET_TOOL = 3
    WLR_INPUT_DEVICE_TABLET_PAD = 4
    WLR_INPUT_DEVICE_SWITCH = 5
end


struct ANONYMOUS20_events
    destroy::wl_signal
end

struct wlr_input_device
    impl::Ptr{wlr_input_device_impl}
    type::wlr_input_device_type
    vendor::UInt32
    product::UInt32
    name::Cstring
    width_mm::Cdouble
    height_mm::Cdouble
    output_name::Cstring
    events::ANONYMOUS20_events
    data::Ptr{Cvoid}
    link::wl_list
end

struct ANONYMOUS82_events
    enter::wl_signal
    leave::wl_signal
end

struct wlr_keyboard_group
    keyboard::wlr_keyboard
    input_device::Ptr{wlr_input_device}
    devices::wl_list
    keys::wl_list
    events::ANONYMOUS82_events
    data::Ptr{Cvoid}
end

struct wlr_keyboard_modifiers
    depressed::xkb_mod_mask_t
    latched::xkb_mod_mask_t
    locked::xkb_mod_mask_t
    group::xkb_mod_mask_t
end

struct ANONYMOUS36_repeat_info
    rate::Int32
    delay::Int32
end

struct ANONYMOUS37_events
    key::wl_signal
    modifiers::wl_signal
    keymap::wl_signal
    repeat_info::wl_signal
    destroy::wl_signal
end

struct wlr_keyboard
    impl::Ptr{wlr_keyboard_impl}
    group::Ptr{wlr_keyboard_group}
    keymap_string::Cstring
    keymap_size::Csize_t
    keymap::Ptr{xkb_keymap}
    xkb_state::Ptr{xkb_state}
    led_indexes::NTuple{3, xkb_led_index_t}
    mod_indexes::NTuple{8, xkb_mod_index_t}
    keycodes::NTuple{32, UInt32}
    num_keycodes::Csize_t
    modifiers::wlr_keyboard_modifiers
    repeat_info::ANONYMOUS36_repeat_info
    events::ANONYMOUS37_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS65_events
    focus_change::wl_signal
end

struct wlr_seat_keyboard_state{wlr_seat<:Abstract_wlr_seat}

    seat::Ptr{wlr_seat}
    keyboard::Ptr{wlr_keyboard}
    focused_client::Ptr{wlr_seat_client}
    focused_surface::Ptr{wlr_surface}
    keyboard_destroy::wl_listener
    keyboard_keymap::wl_listener
    keyboard_repeat_info::wl_listener
    surface_destroy::wl_listener
    grab::Ptr{wlr_seat_keyboard_grab}
    default_grab::Ptr{wlr_seat_keyboard_grab}
    events::ANONYMOUS65_events
end

struct wlr_touch_grab_interface
    down::Ptr{Cvoid}
    up::Ptr{Cvoid}
    motion::Ptr{Cvoid}
    enter::Ptr{Cvoid}
    cancel::Ptr{Cvoid}
end

struct wlr_seat_touch_grab{wlr_seat<:Abstract_wlr_seat}

    interface::Ptr{wlr_touch_grab_interface}
    seat::Ptr{wlr_seat}
    data::Ptr{Cvoid}
end

struct wlr_seat_touch_state{wlr_seat<:Abstract_wlr_seat}

    seat::Ptr{wlr_seat}
    touch_points::wl_list
    grab_serial::UInt32
    grab_id::UInt32
    grab::Ptr{wlr_seat_touch_grab}
    default_grab::Ptr{wlr_seat_touch_grab}
end

struct ANONYMOUS16_events
    pointer_grab_begin::wl_signal
    pointer_grab_end::wl_signal
    keyboard_grab_begin::wl_signal
    keyboard_grab_end::wl_signal
    touch_grab_begin::wl_signal
    touch_grab_end::wl_signal
    request_set_cursor::wl_signal
    request_set_selection::wl_signal
    set_selection::wl_signal
    request_set_primary_selection::wl_signal
    set_primary_selection::wl_signal
    request_start_drag::wl_signal
    start_drag::wl_signal
    destroy::wl_signal
end

struct wlr_seat <: Abstract_wlr_seat

    _global::Ptr{wl_global}
    display::Ptr{wl_display}
    clients::wl_list
    name::Cstring
    capabilities::UInt32
    accumulated_capabilities::UInt32
    last_event::timespec
    selection_source::Ptr{wlr_data_source}
    selection_serial::UInt32
    selection_offers::wl_list
    primary_selection_source::Ptr{wlr_primary_selection_source}
    primary_selection_serial::UInt32
    drag::Ptr{wlr_drag}
    drag_source::Ptr{wlr_data_source}
    drag_serial::UInt32
    drag_offers::wl_list
    pointer_state::wlr_seat_pointer_state
    keyboard_state::wlr_seat_keyboard_state
    touch_state::wlr_seat_touch_state
    display_destroy::wl_listener
    selection_source_destroy::wl_listener
    primary_selection_source_destroy::wl_listener
    drag_source_destroy::wl_listener
    events::ANONYMOUS16_events
    data::Ptr{Cvoid}
end

struct wlr_seat_keyboard_grab
    interface::Ptr{wlr_keyboard_grab_interface}
    seat::Ptr{wlr_seat}
    data::Ptr{Cvoid}
end

struct ANONYMOUS6_events
    map::wl_signal
    unmap::wl_signal
    destroy::wl_signal
end

struct wlr_drag_icon
    drag::Ptr{wlr_drag}
    surface::Ptr{wlr_surface}
    mapped::Bool
    events::ANONYMOUS6_events
    surface_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_drag
    grab_type::wlr_drag_grab_type
    keyboard_grab::wlr_seat_keyboard_grab
    pointer_grab::wlr_seat_pointer_grab
    touch_grab::wlr_seat_touch_grab
    seat::Ptr{wlr_seat}
    seat_client::Ptr{wlr_seat_client}
    focus_client::Ptr{wlr_seat_client}
    icon::Ptr{wlr_drag_icon}
    focus::Ptr{wlr_surface}
    source::Ptr{wlr_data_source}
    started::Bool
    dropped::Bool
    cancelling::Bool
    grab_touch_id::Int32
    touch_id::Int32
    events::ANONYMOUS5_events
    point_destroy::wl_listener
    source_destroy::wl_listener
    seat_client_destroy::wl_listener
    icon_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS7_events
    focus::wl_signal
    motion::wl_signal
    drop::wl_signal
    destroy::wl_signal
end

struct wlr_drag_motion_event
    drag::Ptr{wlr_drag}
    time::UInt32
    sx::Cdouble
    sy::Cdouble
end

struct wlr_drag_drop_event
    drag::Ptr{wlr_drag}
    time::UInt32
end

struct wlr_input_method_v2_preedit_string
    text::Cstring
    cursor_begin::Int32
    cursor_end::Int32
end

struct wlr_input_method_v2_delete_surrounding_text
    before_length::UInt32
    after_length::UInt32
end

struct wlr_input_method_v2_state
    preedit::wlr_input_method_v2_preedit_string
    commit_text::Cstring
    delete::wlr_input_method_v2_delete_surrounding_text
end

struct ANONYMOUS8_events
    commit::wl_signal
    grab_keyboard::wl_signal
    destroy::wl_signal
end

struct ANONYMOUS9_events
    destroy::wl_signal
end

struct wlr_input_method_keyboard_grab_v2
    resource::Ptr{wl_resource}
    input_method::Ptr{wlr_input_method_v2}
    keyboard::Ptr{wlr_keyboard}
    keyboard_keymap::wl_listener
    keyboard_repeat_info::wl_listener
    keyboard_destroy::wl_listener
    events::ANONYMOUS9_events
end

struct wlr_input_method_v2
    resource::Ptr{wl_resource}
    seat::Ptr{wlr_seat}
    seat_client::Ptr{wlr_seat_client}
    pending::wlr_input_method_v2_state
    current::wlr_input_method_v2_state
    active::Bool
    client_active::Bool
    current_serial::UInt32
    keyboard_grab::Ptr{wlr_input_method_keyboard_grab_v2}
    link::wl_list
    seat_client_destroy::wl_listener
    events::ANONYMOUS8_events
end

struct ANONYMOUS10_events
    input_method::wl_signal
    destroy::wl_signal
end

struct wlr_input_method_manager_v2
    _global::Ptr{wl_global}
    input_methods::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS10_events
end

@cenum wlr_xdg_toplevel_decoration_v1_mode::UInt32 begin
    WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE = 0
    WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE = 1
    WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE = 2
end


struct ANONYMOUS11_events
    new_toplevel_decoration::wl_signal
    destroy::wl_signal
end

struct wlr_xdg_decoration_manager_v1
    _global::Ptr{wl_global}
    decorations::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS11_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS40_events
    new_surface::wl_signal
    destroy::wl_signal
end

struct wlr_xdg_shell
    _global::Ptr{wl_global}
    clients::wl_list
    popup_grabs::wl_list
    ping_timeout::UInt32
    display_destroy::wl_listener
    events::ANONYMOUS40_events
    data::Ptr{Cvoid}
end

struct wlr_xdg_client
    shell::Ptr{wlr_xdg_shell}
    resource::Ptr{wl_resource}
    client::Ptr{wl_client}
    surfaces::wl_list
    link::wl_list
    ping_serial::UInt32
    ping_timer::Ptr{wl_event_source}
end

@cenum wlr_xdg_surface_role::UInt32 begin
    WLR_XDG_SURFACE_ROLE_NONE = 0
    WLR_XDG_SURFACE_ROLE_TOPLEVEL = 1
    WLR_XDG_SURFACE_ROLE_POPUP = 2
end


struct wlr_box
    x::Cint
    y::Cint
    width::Cint
    height::Cint
end

struct ANONYMOUS44_events
    destroy::wl_signal
    ping_timeout::wl_signal
    new_popup::wl_signal
    map::wl_signal
    unmap::wl_signal
    configure::wl_signal
    ack_configure::wl_signal
end

struct wlr_xdg_surface
    client::Ptr{wlr_xdg_client}
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    link::wl_list
    role::wlr_xdg_surface_role
    popups::wl_list
    added::Bool
    configured::Bool
    mapped::Bool
    configure_serial::UInt32
    configure_idle::Ptr{wl_event_source}
    configure_next_serial::UInt32
    configure_list::wl_list
    has_next_geometry::Bool
    next_geometry::wlr_box
    geometry::wlr_box
    surface_destroy::wl_listener
    surface_commit::wl_listener
    events::ANONYMOUS44_events
    data::Ptr{Cvoid}
end

struct wlr_output_mode
    width::Int32
    height::Int32
    refresh::Int32
    preferred::Bool
    link::wl_list
end

@cenum wlr_output_adaptive_sync_status::UInt32 begin
    WLR_OUTPUT_ADAPTIVE_SYNC_DISABLED = 0
    WLR_OUTPUT_ADAPTIVE_SYNC_ENABLED = 1
    WLR_OUTPUT_ADAPTIVE_SYNC_UNKNOWN = 2
end

@cenum wlr_output_state_buffer_type::UInt32 begin
    WLR_OUTPUT_STATE_BUFFER_RENDER = 0
    WLR_OUTPUT_STATE_BUFFER_SCANOUT = 1
end

@cenum wlr_output_state_mode_type::UInt32 begin
    WLR_OUTPUT_STATE_MODE_FIXED = 0
    WLR_OUTPUT_STATE_MODE_CUSTOM = 1
end


struct ANONYMOUS93_custom_mode
    width::Int32
    height::Int32
    refresh::Int32
end

struct wlr_output_state
    committed::UInt32
    damage::pixman_region32_t
    enabled::Bool
    scale::Cfloat
    transform::wl_output_transform
    adaptive_sync_enabled::Bool
    buffer_type::wlr_output_state_buffer_type
    buffer::Ptr{wlr_buffer}
    mode_type::wlr_output_state_mode_type
    mode::Ptr{wlr_output_mode}
    custom_mode::ANONYMOUS93_custom_mode
    gamma_lut::Ptr{UInt16}
    gamma_lut_size::Csize_t
end

struct ANONYMOUS94_events
    frame::wl_signal
    damage::wl_signal
    needs_frame::wl_signal
    precommit::wl_signal
    commit::wl_signal
    present::wl_signal
    enable::wl_signal
    mode::wl_signal
    scale::wl_signal
    transform::wl_signal
    description::wl_signal
    destroy::wl_signal
end

struct ANONYMOUS92_events
    destroy::wl_signal
end

struct wlr_output_cursor
    output::Ptr{wlr_output}
    x::Cdouble
    y::Cdouble
    enabled::Bool
    visible::Bool
    width::UInt32
    height::UInt32
    hotspot_x::Int32
    hotspot_y::Int32
    link::wl_list
    texture::Ptr{wlr_texture}
    surface::Ptr{wlr_surface}
    surface_commit::wl_listener
    surface_destroy::wl_listener
    events::ANONYMOUS92_events
end

struct wlr_output
    impl::Ptr{wlr_output_impl}
    backend::Ptr{wlr_backend}
    display::Ptr{wl_display}
    _global::Ptr{wl_global}
    resources::wl_list
    name::NTuple{24, UInt8}
    description::Cstring
    make::NTuple{56, UInt8}
    model::NTuple{16, UInt8}
    serial::NTuple{16, UInt8}
    phys_width::Int32
    phys_height::Int32
    modes::wl_list
    current_mode::Ptr{wlr_output_mode}
    width::Int32
    height::Int32
    refresh::Int32
    enabled::Bool
    scale::Cfloat
    subpixel::wl_output_subpixel
    transform::wl_output_transform
    adaptive_sync_status::wlr_output_adaptive_sync_status
    needs_frame::Bool
    frame_pending::Bool
    transform_matrix::NTuple{9, Cfloat}
    pending::wlr_output_state
    commit_seq::UInt32
    events::ANONYMOUS94_events
    idle_frame::Ptr{wl_event_source}
    idle_done::Ptr{wl_event_source}
    attach_render_locks::Cint
    cursors::wl_list
    hardware_cursor::Ptr{wlr_output_cursor}
    software_cursor_locks::Cint
    display_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_xdg_toplevel_state
    maximized::Bool
    fullscreen::Bool
    resizing::Bool
    activated::Bool
    tiled::UInt32
    width::UInt32
    height::UInt32
    max_width::UInt32
    max_height::UInt32
    min_width::UInt32
    min_height::UInt32
    fullscreen_output::Ptr{wlr_output}
    fullscreen_output_destroy::wl_listener
end

struct wlr_xdg_surface_configure
    surface::Ptr{wlr_xdg_surface}
    link::wl_list
    serial::UInt32
    toplevel_state::Ptr{wlr_xdg_toplevel_state}
end

struct wlr_xdg_toplevel_decoration_v1_configure
    link::wl_list
    surface_configure::Ptr{wlr_xdg_surface_configure}
    mode::wlr_xdg_toplevel_decoration_v1_mode
end

struct ANONYMOUS12_events
    destroy::wl_signal
    request_mode::wl_signal
end

struct wlr_xdg_toplevel_decoration_v1
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_xdg_surface}
    manager::Ptr{wlr_xdg_decoration_manager_v1}
    link::wl_list
    added::Bool
    current_mode::wlr_xdg_toplevel_decoration_v1_mode
    client_pending_mode::wlr_xdg_toplevel_decoration_v1_mode
    server_pending_mode::wlr_xdg_toplevel_decoration_v1_mode
    configure_list::wl_list
    events::ANONYMOUS12_events
    surface_destroy::wl_listener
    surface_configure::wl_listener
    surface_ack_configure::wl_listener
    surface_commit::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS14_events
    destroy::wl_signal
end

struct wlr_list
    capacity::Csize_t
    length::Csize_t
    items::Ptr{Ptr{Cvoid}}
end

struct wlr_output_event_present
    output::Ptr{wlr_output}
    commit_seq::UInt32
    when::Ptr{timespec}
    seq::UInt32
    refresh::Cint
    flags::UInt32
end

struct ANONYMOUS15_events
    destroy::wl_signal
end

struct wlr_presentation
    _global::Ptr{wl_global}
    feedbacks::wl_list
    clock::clockid_t
    events::ANONYMOUS15_events
    display_destroy::wl_listener
end

struct wlr_presentation_feedback
    presentation::Ptr{wlr_presentation}
    surface::Ptr{wlr_surface}
    link::wl_list
    resources::wl_list
    committed::Bool
    sampled::Bool
    presented::Bool
    output::Ptr{wlr_output}
    output_committed::Bool
    output_commit_seq::UInt32
    surface_commit::wl_listener
    surface_destroy::wl_listener
    output_commit::wl_listener
    output_present::wl_listener
    output_destroy::wl_listener
end

struct wlr_presentation_event
    output::Ptr{wlr_output}
    tv_sec::UInt64
    tv_nsec::UInt32
    refresh::UInt32
    seq::UInt64
    flags::UInt32
end

@cenum wlr_pointer_constraint_v1_type::UInt32 begin
    WLR_POINTER_CONSTRAINT_V1_LOCKED = 0
    WLR_POINTER_CONSTRAINT_V1_CONFINED = 1
end

@cenum wlr_pointer_constraint_v1_state_field::UInt32 begin
    WLR_POINTER_CONSTRAINT_V1_STATE_REGION = 1
    WLR_POINTER_CONSTRAINT_V1_STATE_CURSOR_HINT = 2
end


struct ANONYMOUS17_cursor_hint
    x::Cdouble
    y::Cdouble
end

struct wlr_pointer_constraint_v1_state
    committed::UInt32
    region::pixman_region32_t
    cursor_hint::ANONYMOUS17_cursor_hint
end

struct ANONYMOUS18_events
    new_constraint::wl_signal
end

struct wlr_pointer_constraints_v1
    _global::Ptr{wl_global}
    constraints::wl_list
    events::ANONYMOUS18_events
    display_destroy::wl_listener
    data::Ptr{Cvoid}
end

const WLR_OUTPUT_DAMAGE_PREVIOUS_LEN = 2

struct ANONYMOUS19_events
    frame::wl_signal
    destroy::wl_signal
end

struct wlr_output_damage
    output::Ptr{wlr_output}
    max_rects::Cint
    current::pixman_region32_t
    previous::NTuple{2, pixman_region32_t}
    previous_idx::Csize_t
    events::ANONYMOUS19_events
    output_destroy::wl_listener
    output_mode::wl_listener
    output_transform::wl_listener
    output_scale::wl_listener
    output_needs_frame::wl_listener
    output_damage::wl_listener
    output_frame::wl_listener
    output_commit::wl_listener
end

@cenum wlr_button_state::UInt32 begin
    WLR_BUTTON_RELEASED = 0
    WLR_BUTTON_PRESSED = 1
end


struct ANONYMOUS21_events
    new_virtual_pointer::wl_signal
    destroy::wl_signal
end

struct wlr_virtual_pointer_manager_v1
    _global::Ptr{wl_global}
    virtual_pointers::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS21_events
end

struct ANONYMOUS22_events
    destroy::wl_signal
end

@cenum wlr_axis_source::UInt32 begin
    WLR_AXIS_SOURCE_WHEEL = 0
    WLR_AXIS_SOURCE_FINGER = 1
    WLR_AXIS_SOURCE_CONTINUOUS = 2
    WLR_AXIS_SOURCE_WHEEL_TILT = 3
end

@cenum wlr_axis_orientation::UInt32 begin
    WLR_AXIS_ORIENTATION_VERTICAL = 0
    WLR_AXIS_ORIENTATION_HORIZONTAL = 1
end


struct wlr_event_pointer_axis
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    source::wlr_axis_source
    orientation::wlr_axis_orientation
    delta::Cdouble
    delta_discrete::Int32
end

struct wlr_virtual_pointer_v1
    input_device::wlr_input_device
    resource::Ptr{wl_resource}
    axis_event::NTuple{2, wlr_event_pointer_axis}
    axis::wl_pointer_axis
    axis_valid::NTuple{2, Bool}
    link::wl_list
    events::ANONYMOUS22_events
end

struct wlr_virtual_pointer_v1_new_pointer_event
    new_pointer::Ptr{wlr_virtual_pointer_v1}
    suggested_seat::Ptr{wlr_seat}
    suggested_output::Ptr{wlr_output}
end

struct ANONYMOUS23_events
    destroy::wl_signal
end

struct wlr_foreign_toplevel_manager_v1
    event_loop::Ptr{wl_event_loop}
    _global::Ptr{wl_global}
    resources::wl_list
    toplevels::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS23_events
    data::Ptr{Cvoid}
end

@cenum wlr_foreign_toplevel_handle_v1_state::UInt32 begin
    WLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED = 1
    WLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED = 2
    WLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED = 4
    WLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN = 8
end


struct ANONYMOUS24_events
    request_maximize::wl_signal
    request_minimize::wl_signal
    request_activate::wl_signal
    request_fullscreen::wl_signal
    request_close::wl_signal
    set_rectangle::wl_signal
    destroy::wl_signal
end

struct wlr_foreign_toplevel_handle_v1
    manager::Ptr{wlr_foreign_toplevel_manager_v1}
    resources::wl_list
    link::wl_list
    idle_source::Ptr{wl_event_source}
    title::Cstring
    app_id::Cstring
    outputs::wl_list
    state::UInt32
    events::ANONYMOUS24_events
    data::Ptr{Cvoid}
end

struct wlr_foreign_toplevel_handle_v1_output
    link::wl_list
    output_destroy::wl_listener
    output::Ptr{wlr_output}
    toplevel::Ptr{wlr_foreign_toplevel_handle_v1}
end

struct wlr_foreign_toplevel_handle_v1_maximized_event
    toplevel::Ptr{wlr_foreign_toplevel_handle_v1}
    maximized::Bool
end

struct wlr_foreign_toplevel_handle_v1_minimized_event
    toplevel::Ptr{wlr_foreign_toplevel_handle_v1}
    minimized::Bool
end

struct wlr_foreign_toplevel_handle_v1_activated_event
    toplevel::Ptr{wlr_foreign_toplevel_handle_v1}
    seat::Ptr{wlr_seat}
end

struct wlr_foreign_toplevel_handle_v1_fullscreen_event
    toplevel::Ptr{wlr_foreign_toplevel_handle_v1}
    fullscreen::Bool
    output::Ptr{wlr_output}
end

struct wlr_foreign_toplevel_handle_v1_set_rectangle_event
    toplevel::Ptr{wlr_foreign_toplevel_handle_v1}
    surface::Ptr{wlr_surface}
    x::Int32
    y::Int32
    width::Int32
    height::Int32
end

@cenum wlr_server_decoration_manager_mode::UInt32 begin
    WLR_SERVER_DECORATION_MANAGER_MODE_NONE = 0
    WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT = 1
    WLR_SERVER_DECORATION_MANAGER_MODE_SERVER = 2
end


struct ANONYMOUS25_events
    new_decoration::wl_signal
    destroy::wl_signal
end

struct wlr_server_decoration_manager
    _global::Ptr{wl_global}
    resources::wl_list
    decorations::wl_list
    default_mode::UInt32
    display_destroy::wl_listener
    events::ANONYMOUS25_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS26_events
    destroy::wl_signal
    mode::wl_signal
end

struct wlr_server_decoration
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    link::wl_list
    mode::UInt32
    events::ANONYMOUS26_events
    surface_destroy_listener::wl_listener
    data::Ptr{Cvoid}
end

@cenum wlr_surface_state_field::UInt32 begin
    WLR_SURFACE_STATE_BUFFER = 1
    WLR_SURFACE_STATE_SURFACE_DAMAGE = 2
    WLR_SURFACE_STATE_BUFFER_DAMAGE = 4
    WLR_SURFACE_STATE_OPAQUE_REGION = 8
    WLR_SURFACE_STATE_INPUT_REGION = 16
    WLR_SURFACE_STATE_TRANSFORM = 32
    WLR_SURFACE_STATE_SCALE = 64
    WLR_SURFACE_STATE_FRAME_CALLBACK_LIST = 128
    WLR_SURFACE_STATE_VIEWPORT = 256
end


struct wlr_subsurface_state
    x::Int32
    y::Int32
end

struct ANONYMOUS29_events
    destroy::wl_signal
    map::wl_signal
    unmap::wl_signal
end

struct wlr_subsurface
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    parent::Ptr{wlr_surface}
    current::wlr_subsurface_state
    pending::wlr_subsurface_state
    cached::wlr_surface_state
    has_cache::Bool
    synchronized::Bool
    reordered::Bool
    mapped::Bool
    parent_link::wl_list
    parent_pending_link::wl_list
    surface_destroy::wl_listener
    parent_destroy::wl_listener
    events::ANONYMOUS29_events
    data::Ptr{Cvoid}
end

const wlr_surface_iterator_func_t = Ptr{Cvoid}

struct ANONYMOUS31_events
    destroy::wl_signal
    release::wl_signal
end

struct ANONYMOUS32_events
    destroy::wl_signal
end

struct wlr_gamma_control_manager_v1
    _global::Ptr{wl_global}
    controls::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS32_events
    data::Ptr{Cvoid}
end

struct wlr_gamma_control_v1
    resource::Ptr{wl_resource}
    output::Ptr{wlr_output}
    link::wl_list
    output_destroy_listener::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS33_events
    new_surface::wl_signal
    destroy::wl_signal
end

struct wlr_xdg_shell_v6
    _global::Ptr{wl_global}
    clients::wl_list
    popup_grabs::wl_list
    ping_timeout::UInt32
    display_destroy::wl_listener
    events::ANONYMOUS33_events
    data::Ptr{Cvoid}
end

struct wlr_xdg_client_v6
    shell::Ptr{wlr_xdg_shell_v6}
    resource::Ptr{wl_resource}
    client::Ptr{wl_client}
    surfaces::wl_list
    link::wl_list
    ping_serial::UInt32
    ping_timer::Ptr{wl_event_source}
end

@cenum wlr_xdg_surface_v6_role::UInt32 begin
    WLR_XDG_SURFACE_V6_ROLE_NONE = 0
    WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL = 1
    WLR_XDG_SURFACE_V6_ROLE_POPUP = 2
end


struct ANONYMOUS35_events
    destroy::wl_signal
    ping_timeout::wl_signal
    new_popup::wl_signal
    map::wl_signal
    unmap::wl_signal
end

struct wlr_xdg_surface_v6
    client::Ptr{wlr_xdg_client_v6}
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    link::wl_list
    role::wlr_xdg_surface_v6_role
    popups::wl_list
    added::Bool
    configured::Bool
    mapped::Bool
    configure_serial::UInt32
    configure_idle::Ptr{wl_event_source}
    configure_next_serial::UInt32
    configure_list::wl_list
    has_next_geometry::Bool
    next_geometry::wlr_box
    geometry::wlr_box
    surface_destroy::wl_listener
    surface_commit::wl_listener
    events::ANONYMOUS35_events
    data::Ptr{Cvoid}
end

struct wlr_xdg_popup_v6
    base::Ptr{wlr_xdg_surface_v6}
    link::wl_list
    resource::Ptr{wl_resource}
    committed::Bool
    parent::Ptr{wlr_xdg_surface_v6}
    seat::Ptr{wlr_seat}
    geometry::wlr_box
    positioner::wlr_xdg_positioner_v6
    grab_link::wl_list
end

struct wlr_xdg_popup_grab_v6
    client::Ptr{wl_client}
    pointer_grab::wlr_seat_pointer_grab
    keyboard_grab::wlr_seat_keyboard_grab
    touch_grab::wlr_seat_touch_grab
    seat::Ptr{wlr_seat}
    popups::wl_list
    link::wl_list
    seat_destroy::wl_listener
end

struct wlr_xdg_toplevel_v6_state
    maximized::Bool
    fullscreen::Bool
    resizing::Bool
    activated::Bool
    width::UInt32
    height::UInt32
    max_width::UInt32
    max_height::UInt32
    min_width::UInt32
    min_height::UInt32
    fullscreen_output::Ptr{wlr_output}
    fullscreen_output_destroy::wl_listener
end

struct ANONYMOUS34_events
    request_maximize::wl_signal
    request_fullscreen::wl_signal
    request_minimize::wl_signal
    request_move::wl_signal
    request_resize::wl_signal
    request_show_window_menu::wl_signal
    set_parent::wl_signal
    set_title::wl_signal
    set_app_id::wl_signal
end

struct wlr_xdg_toplevel_v6
    resource::Ptr{wl_resource}
    base::Ptr{wlr_xdg_surface_v6}
    parent::Ptr{wlr_xdg_surface_v6}
    added::Bool
    client_pending::wlr_xdg_toplevel_v6_state
    server_pending::wlr_xdg_toplevel_v6_state
    current::wlr_xdg_toplevel_v6_state
    title::Cstring
    app_id::Cstring
    events::ANONYMOUS34_events
end

struct wlr_xdg_surface_v6_configure
    link::wl_list
    serial::UInt32
    toplevel_state::Ptr{wlr_xdg_toplevel_v6_state}
end

struct wlr_xdg_toplevel_v6_move_event
    surface::Ptr{wlr_xdg_surface_v6}
    seat::Ptr{wlr_seat_client}
    serial::UInt32
end

struct wlr_xdg_toplevel_v6_resize_event
    surface::Ptr{wlr_xdg_surface_v6}
    seat::Ptr{wlr_seat_client}
    serial::UInt32
    edges::UInt32
end

struct wlr_xdg_toplevel_v6_set_fullscreen_event
    surface::Ptr{wlr_xdg_surface_v6}
    fullscreen::Bool
    output::Ptr{wlr_output}
end

struct wlr_xdg_toplevel_v6_show_window_menu_event
    surface::Ptr{wlr_xdg_surface_v6}
    seat::Ptr{wlr_seat_client}
    serial::UInt32
    x::UInt32
    y::UInt32
end

const WLR_LED_COUNT = 3
const WLR_MODIFIER_COUNT = 8
const WLR_KEYBOARD_KEYS_CAP = 32

@cenum wlr_keyboard_led::UInt32 begin
    WLR_LED_NUM_LOCK = 1
    WLR_LED_CAPS_LOCK = 2
    WLR_LED_SCROLL_LOCK = 4
end

@cenum wlr_keyboard_modifier::UInt32 begin
    WLR_MODIFIER_SHIFT = 1
    WLR_MODIFIER_CAPS = 2
    WLR_MODIFIER_CTRL = 4
    WLR_MODIFIER_ALT = 8
    WLR_MODIFIER_MOD2 = 16
    WLR_MODIFIER_MOD3 = 32
    WLR_MODIFIER_LOGO = 64
    WLR_MODIFIER_MOD5 = 128
end

@cenum wlr_key_state::UInt32 begin
    WLR_KEY_RELEASED = 0
    WLR_KEY_PRESSED = 1
end


struct wlr_event_keyboard_key
    time_msec::UInt32
    keycode::UInt32
    update_state::Bool
    state::wlr_key_state
end

struct ANONYMOUS38_events
    down::wl_signal
    up::wl_signal
    motion::wl_signal
    cancel::wl_signal
end

struct wlr_touch
    impl::Ptr{wlr_touch_impl}
    events::ANONYMOUS38_events
    data::Ptr{Cvoid}
end

struct wlr_event_touch_down
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
    x::Cdouble
    y::Cdouble
end

struct wlr_event_touch_up
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
end

struct wlr_event_touch_motion
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
    x::Cdouble
    y::Cdouble
end

struct wlr_event_touch_cancel
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    touch_id::Int32
end

const wlr_cursor_state = Cvoid

struct ANONYMOUS39_events
    motion::wl_signal
    motion_absolute::wl_signal
    button::wl_signal
    axis::wl_signal
    frame::wl_signal
    swipe_begin::wl_signal
    swipe_update::wl_signal
    swipe_end::wl_signal
    pinch_begin::wl_signal
    pinch_update::wl_signal
    pinch_end::wl_signal
    touch_up::wl_signal
    touch_down::wl_signal
    touch_motion::wl_signal
    touch_cancel::wl_signal
    tablet_tool_axis::wl_signal
    tablet_tool_proximity::wl_signal
    tablet_tool_tip::wl_signal
    tablet_tool_button::wl_signal
end

struct wlr_cursor
    state::Ptr{wlr_cursor_state}
    x::Cdouble
    y::Cdouble
    events::ANONYMOUS39_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS41_size
    width::Int32
    height::Int32
end

struct ANONYMOUS42_offset
    x::Int32
    y::Int32
end

struct wlr_xdg_positioner
    resource::Ptr{wl_resource}
    anchor_rect::wlr_box
    anchor::xdg_positioner_anchor
    gravity::xdg_positioner_gravity
    constraint_adjustment::xdg_positioner_constraint_adjustment
    size::ANONYMOUS41_size
    offset::ANONYMOUS42_offset
end

struct wlr_xdg_popup
    base::Ptr{wlr_xdg_surface}
    link::wl_list
    resource::Ptr{wl_resource}
    committed::Bool
    parent::Ptr{wlr_surface}
    seat::Ptr{wlr_seat}
    geometry::wlr_box
    positioner::wlr_xdg_positioner
    grab_link::wl_list
end

struct wlr_xdg_popup_grab
    client::Ptr{wl_client}
    pointer_grab::wlr_seat_pointer_grab
    keyboard_grab::wlr_seat_keyboard_grab
    touch_grab::wlr_seat_touch_grab
    seat::Ptr{wlr_seat}
    popups::wl_list
    link::wl_list
    seat_destroy::wl_listener
end

struct ANONYMOUS43_events
    request_maximize::wl_signal
    request_fullscreen::wl_signal
    request_minimize::wl_signal
    request_move::wl_signal
    request_resize::wl_signal
    request_show_window_menu::wl_signal
    set_parent::wl_signal
    set_title::wl_signal
    set_app_id::wl_signal
end

struct wlr_xdg_toplevel
    resource::Ptr{wl_resource}
    base::Ptr{wlr_xdg_surface}
    added::Bool
    parent::Ptr{wlr_xdg_surface}
    parent_unmap::wl_listener
    client_pending::wlr_xdg_toplevel_state
    server_pending::wlr_xdg_toplevel_state
    current::wlr_xdg_toplevel_state
    title::Cstring
    app_id::Cstring
    events::ANONYMOUS43_events
end

struct wlr_xdg_toplevel_move_event
    surface::Ptr{wlr_xdg_surface}
    seat::Ptr{wlr_seat_client}
    serial::UInt32
end

struct wlr_xdg_toplevel_resize_event
    surface::Ptr{wlr_xdg_surface}
    seat::Ptr{wlr_seat_client}
    serial::UInt32
    edges::UInt32
end

struct wlr_xdg_toplevel_set_fullscreen_event
    surface::Ptr{wlr_xdg_surface}
    fullscreen::Bool
    output::Ptr{wlr_output}
end

struct wlr_xdg_toplevel_show_window_menu_event
    surface::Ptr{wlr_xdg_surface}
    seat::Ptr{wlr_seat_client}
    serial::UInt32
    x::UInt32
    y::UInt32
end

struct ANONYMOUS45_events
    toggle::wl_signal
end

struct wlr_switch
    impl::Ptr{wlr_switch_impl}
    events::ANONYMOUS45_events
    data::Ptr{Cvoid}
end

@cenum wlr_switch_type::UInt32 begin
    WLR_SWITCH_TYPE_LID = 1
    WLR_SWITCH_TYPE_TABLET_MODE = 2
end

@cenum wlr_switch_state::UInt32 begin
    WLR_SWITCH_STATE_OFF = 0
    WLR_SWITCH_STATE_ON = 1
    WLR_SWITCH_STATE_TOGGLE = 2
end


struct wlr_event_switch_toggle
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    switch_type::wlr_switch_type
    switch_state::wlr_switch_state
end

struct ANONYMOUS46_events
    destroy::wl_signal
end

struct wlr_export_dmabuf_manager_v1
    _global::Ptr{wl_global}
    frames::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS46_events
end

struct wlr_dmabuf_attributes
    width::Int32
    height::Int32
    format::UInt32
    flags::UInt32
    modifier::UInt64
    n_planes::Cint
    offset::NTuple{4, UInt32}
    stride::NTuple{4, UInt32}
    fd::NTuple{4, Cint}
end

struct wlr_export_dmabuf_frame_v1
    resource::Ptr{wl_resource}
    manager::Ptr{wlr_export_dmabuf_manager_v1}
    link::wl_list
    attribs::wlr_dmabuf_attributes
    output::Ptr{wlr_output}
    cursor_locked::Bool
    output_precommit::wl_listener
end

const wlr_output_layout_state = Cvoid

struct ANONYMOUS49_events
    add::wl_signal
    change::wl_signal
    destroy::wl_signal
end

struct wlr_output_layout
    outputs::wl_list
    state::Ptr{wlr_output_layout_state}
    events::ANONYMOUS49_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS47_events
    destroy::wl_signal
end

struct wlr_xdg_output_manager_v1
    _global::Ptr{wl_global}
    layout::Ptr{wlr_output_layout}
    outputs::wl_list
    events::ANONYMOUS47_events
    display_destroy::wl_listener
    layout_add::wl_listener
    layout_change::wl_listener
    layout_destroy::wl_listener
end

const wlr_output_layout_output_state = Cvoid

struct ANONYMOUS50_events
    destroy::wl_signal
end

struct wlr_output_layout_output
    output::Ptr{wlr_output}
    x::Cint
    y::Cint
    link::wl_list
    state::Ptr{wlr_output_layout_output_state}
    events::ANONYMOUS50_events
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

struct ANONYMOUS48_events
    destroy::wl_signal
    new_device::wl_signal
end

struct wlr_data_control_manager_v1
    _global::Ptr{wl_global}
    devices::wl_list
    events::ANONYMOUS48_events
    display_destroy::wl_listener
end

struct wlr_data_control_device_v1
    resource::Ptr{wl_resource}
    manager::Ptr{wlr_data_control_manager_v1}
    link::wl_list
    seat::Ptr{wlr_seat}
    selection_offer_resource::Ptr{wl_resource}
    primary_selection_offer_resource::Ptr{wl_resource}
    seat_destroy::wl_listener
    seat_set_selection::wl_listener
    seat_set_primary_selection::wl_listener
end

@cenum wlr_direction::UInt32 begin
    WLR_DIRECTION_UP = 1
    WLR_DIRECTION_DOWN = 2
    WLR_DIRECTION_LEFT = 4
    WLR_DIRECTION_RIGHT = 8
end


struct wlr_xcursor_image
    width::UInt32
    height::UInt32
    hotspot_x::UInt32
    hotspot_y::UInt32
    delay::UInt32
    buffer::Ptr{UInt8}
end

struct wlr_xcursor
    image_count::UInt32
    images::Ptr{Ptr{wlr_xcursor_image}}
    name::Cstring
    total_delay::UInt32
end

struct wlr_xcursor_theme
    cursor_count::UInt32
    cursors::Ptr{Ptr{wlr_xcursor}}
    name::Cstring
    size::Cint
end

struct wlr_xcursor_manager_theme
    scale::Cfloat
    theme::Ptr{wlr_xcursor_theme}
    link::wl_list
end

struct wlr_xcursor_manager
    name::Cstring
    size::UInt32
    scaled_themes::wl_list
end

struct ANONYMOUS51_events
    apply::wl_signal
    test::wl_signal
    destroy::wl_signal
end

struct wlr_output_manager_v1
    display::Ptr{wl_display}
    _global::Ptr{wl_global}
    resources::wl_list
    heads::wl_list
    serial::UInt32
    current_configuration_dirty::Bool
    events::ANONYMOUS51_events
    display_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS52_custom_mode
    width::Cint
    height::Cint
    refresh::Cint
end

struct wlr_output_head_v1_state
    output::Ptr{wlr_output}
    enabled::Bool
    mode::Ptr{wlr_output_mode}
    custom_mode::ANONYMOUS52_custom_mode
    x::Int32
    y::Int32
    transform::wl_output_transform
    scale::Cdouble
end

struct wlr_output_head_v1
    state::wlr_output_head_v1_state
    manager::Ptr{wlr_output_manager_v1}
    link::wl_list
    resources::wl_list
    mode_resources::wl_list
    output_destroy::wl_listener
end

struct wlr_output_configuration_v1
    heads::wl_list
    manager::Ptr{wlr_output_manager_v1}
    serial::UInt32
    finalized::Bool
    finished::Bool
    resource::Ptr{wl_resource}
end

struct wlr_output_configuration_head_v1
    state::wlr_output_head_v1_state
    config::Ptr{wlr_output_configuration_v1}
    link::wl_list
    resource::Ptr{wl_resource}
    output_destroy::wl_listener
end

struct ANONYMOUS53_events
    destroy::wl_signal
end

struct wlr_screencopy_manager_v1
    _global::Ptr{wl_global}
    frames::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS53_events
    data::Ptr{Cvoid}
end

struct wlr_screencopy_v1_client
    ref::Cint
    manager::Ptr{wlr_screencopy_manager_v1}
    damages::wl_list
end

struct wlr_dmabuf_v1_buffer
    renderer::Ptr{wlr_renderer}
    buffer_resource::Ptr{wl_resource}
    params_resource::Ptr{wl_resource}
    attributes::wlr_dmabuf_attributes
    has_modifier::Bool
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

struct ANONYMOUS54_events
    new_inhibitor::wl_signal
    destroy::wl_signal
end

struct wlr_idle_inhibit_manager_v1
    inhibitors::wl_list
    _global::Ptr{wl_global}
    display_destroy::wl_listener
    events::ANONYMOUS54_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS55_events
    destroy::wl_signal
end

struct wlr_idle_inhibitor_v1
    surface::Ptr{wlr_surface}
    resource::Ptr{wl_resource}
    surface_destroy::wl_listener
    link::wl_list
    events::ANONYMOUS55_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS56_events
    destroy::wl_signal
end

struct wlr_viewporter
    _global::Ptr{wl_global}
    events::ANONYMOUS56_events
    display_destroy::wl_listener
end

struct wlr_viewport
    resource::Ptr{wl_resource}
    surface::Ptr{wlr_surface}
    surface_destroy::wl_listener
    surface_commit::wl_listener
end

struct ANONYMOUS57_events
    destroy::wl_signal
end

struct wlr_primary_selection_v1_device_manager
    _global::Ptr{wl_global}
    devices::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS57_events
    data::Ptr{Cvoid}
end

struct wlr_primary_selection_v1_device
    manager::Ptr{wlr_primary_selection_v1_device_manager}
    seat::Ptr{wlr_seat}
    link::wl_list
    resources::wl_list
    offers::wl_list
    seat_destroy::wl_listener
    seat_focus_change::wl_listener
    seat_set_primary_selection::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS58_events
    new_virtual_keyboard::wl_signal
    destroy::wl_signal
end

struct wlr_virtual_keyboard_manager_v1
    _global::Ptr{wl_global}
    virtual_keyboards::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS58_events
end

struct ANONYMOUS59_events
    destroy::wl_signal
end

struct wlr_virtual_keyboard_v1
    input_device::wlr_input_device
    resource::Ptr{wl_resource}
    seat::Ptr{wlr_seat}
    has_keymap::Bool
    link::wl_list
    events::ANONYMOUS59_events
end

@cenum wlr_tablet_tool_type::UInt32 begin
    WLR_TABLET_TOOL_TYPE_PEN = 1
    WLR_TABLET_TOOL_TYPE_ERASER = 2
    WLR_TABLET_TOOL_TYPE_BRUSH = 3
    WLR_TABLET_TOOL_TYPE_PENCIL = 4
    WLR_TABLET_TOOL_TYPE_AIRBRUSH = 5
    WLR_TABLET_TOOL_TYPE_MOUSE = 6
    WLR_TABLET_TOOL_TYPE_LENS = 7
    WLR_TABLET_TOOL_TYPE_TOTEM = 8
end


struct ANONYMOUS60_events
    destroy::wl_signal
end

struct wlr_tablet_tool
    type::wlr_tablet_tool_type
    hardware_serial::UInt64
    hardware_wacom::UInt64
    tilt::Bool
    pressure::Bool
    distance::Bool
    rotation::Bool
    slider::Bool
    wheel::Bool
    events::ANONYMOUS60_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS61_events
    axis::wl_signal
    proximity::wl_signal
    tip::wl_signal
    button::wl_signal
end

struct wlr_tablet
    impl::Ptr{wlr_tablet_impl}
    events::ANONYMOUS61_events
    name::Cstring
    paths::wlr_list
    data::Ptr{Cvoid}
end

@cenum wlr_tablet_tool_axes::UInt32 begin
    WLR_TABLET_TOOL_AXIS_X = 1
    WLR_TABLET_TOOL_AXIS_Y = 2
    WLR_TABLET_TOOL_AXIS_DISTANCE = 4
    WLR_TABLET_TOOL_AXIS_PRESSURE = 8
    WLR_TABLET_TOOL_AXIS_TILT_X = 16
    WLR_TABLET_TOOL_AXIS_TILT_Y = 32
    WLR_TABLET_TOOL_AXIS_ROTATION = 64
    WLR_TABLET_TOOL_AXIS_SLIDER = 128
    WLR_TABLET_TOOL_AXIS_WHEEL = 256
end


struct wlr_event_tablet_tool_axis
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    updated_axes::UInt32
    x::Cdouble
    y::Cdouble
    dx::Cdouble
    dy::Cdouble
    pressure::Cdouble
    distance::Cdouble
    tilt_x::Cdouble
    tilt_y::Cdouble
    rotation::Cdouble
    slider::Cdouble
    wheel_delta::Cdouble
end

@cenum wlr_tablet_tool_proximity_state::UInt32 begin
    WLR_TABLET_TOOL_PROXIMITY_OUT = 0
    WLR_TABLET_TOOL_PROXIMITY_IN = 1
end


struct wlr_event_tablet_tool_proximity
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    x::Cdouble
    y::Cdouble
    state::wlr_tablet_tool_proximity_state
end

@cenum wlr_tablet_tool_tip_state::UInt32 begin
    WLR_TABLET_TOOL_TIP_UP = 0
    WLR_TABLET_TOOL_TIP_DOWN = 1
end


struct wlr_event_tablet_tool_tip
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    x::Cdouble
    y::Cdouble
    state::wlr_tablet_tool_tip_state
end

struct wlr_event_tablet_tool_button
    device::Ptr{wlr_input_device}
    tool::Ptr{wlr_tablet_tool}
    time_msec::UInt32
    button::UInt32
    state::wlr_button_state
end

const WLR_SERIAL_RINGSET_SIZE = 128
const WLR_POINTER_BUTTONS_CAP = 16

struct ANONYMOUS63_events
    destroy::wl_signal
end

struct wlr_touch_point
    touch_id::Int32
    surface::Ptr{wlr_surface}
    client::Ptr{wlr_seat_client}
    focus_surface::Ptr{wlr_surface}
    focus_client::Ptr{wlr_seat_client}
    sx::Cdouble
    sy::Cdouble
    surface_destroy::wl_listener
    focus_surface_destroy::wl_listener
    client_destroy::wl_listener
    events::ANONYMOUS63_events
    link::wl_list
end

struct ANONYMOUS66_events
    pointer_grab_begin::wl_signal
    pointer_grab_end::wl_signal
    keyboard_grab_begin::wl_signal
    keyboard_grab_end::wl_signal
    touch_grab_begin::wl_signal
    touch_grab_end::wl_signal
    request_set_cursor::wl_signal
    request_set_selection::wl_signal
    set_selection::wl_signal
    request_set_primary_selection::wl_signal
    set_primary_selection::wl_signal
    request_start_drag::wl_signal
    start_drag::wl_signal
    destroy::wl_signal
end

struct wlr_seat_pointer_request_set_cursor_event
    seat_client::Ptr{wlr_seat_client}
    surface::Ptr{wlr_surface}
    serial::UInt32
    hotspot_x::Int32
    hotspot_y::Int32
end

struct wlr_seat_request_set_selection_event
    source::Ptr{wlr_data_source}
    serial::UInt32
end

struct wlr_seat_request_set_primary_selection_event
    source::Ptr{wlr_primary_selection_source}
    serial::UInt32
end

struct wlr_seat_request_start_drag_event
    drag::Ptr{wlr_drag}
    origin::Ptr{wlr_surface}
    serial::UInt32
end

struct wlr_seat_pointer_focus_change_event
    seat::Ptr{wlr_seat}
    old_surface::Ptr{wlr_surface}
    new_surface::Ptr{wlr_surface}
    sx::Cdouble
    sy::Cdouble
end

struct wlr_seat_keyboard_focus_change_event
    seat::Ptr{wlr_seat}
    old_surface::Ptr{wlr_surface}
    new_surface::Ptr{wlr_surface}
end

struct ANONYMOUS67_events
    destroy::wl_signal
    present_surface::wl_signal
end

struct wlr_fullscreen_shell_v1
    _global::Ptr{wl_global}
    events::ANONYMOUS67_events
    display_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS68_events
    destroy::wl_signal
end

struct wlr_gtk_primary_selection_device_manager
    _global::Ptr{wl_global}
    devices::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS68_events
    data::Ptr{Cvoid}
end

struct wlr_gtk_primary_selection_device
    manager::Ptr{wlr_gtk_primary_selection_device_manager}
    seat::Ptr{wlr_seat}
    link::wl_list
    resources::wl_list
    offers::wl_list
    seat_destroy::wl_listener
    seat_focus_change::wl_listener
    seat_set_primary_selection::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS69_events
    destroy::wl_signal
    new_relative_pointer::wl_signal
end

struct wlr_relative_pointer_manager_v1
    _global::Ptr{wl_global}
    relative_pointers::wl_list
    events::ANONYMOUS69_events
    display_destroy_listener::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS70_events
    destroy::wl_signal
end

struct wlr_relative_pointer_v1
    resource::Ptr{wl_resource}
    pointer_resource::Ptr{wl_resource}
    seat::Ptr{wlr_seat}
    link::wl_list
    events::ANONYMOUS70_events
    seat_destroy::wl_listener
    pointer_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct ANONYMOUS71_events
    new_inhibitor::wl_signal
    destroy::wl_signal
end

struct wlr_keyboard_shortcuts_inhibit_manager_v1
    inhibitors::wl_list
    _global::Ptr{wl_global}
    display_destroy::wl_listener
    events::ANONYMOUS71_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS72_events
    destroy::wl_signal
end

struct wlr_keyboard_shortcuts_inhibitor_v1
    surface::Ptr{wlr_surface}
    seat::Ptr{wlr_seat}
    active::Bool
    resource::Ptr{wl_resource}
    surface_destroy::wl_listener
    seat_destroy::wl_listener
    link::wl_list
    events::ANONYMOUS72_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS73_events
    motion::wl_signal
    motion_absolute::wl_signal
    button::wl_signal
    axis::wl_signal
    frame::wl_signal
    swipe_begin::wl_signal
    swipe_update::wl_signal
    swipe_end::wl_signal
    pinch_begin::wl_signal
    pinch_update::wl_signal
    pinch_end::wl_signal
end

struct wlr_pointer
    impl::Ptr{wlr_pointer_impl}
    events::ANONYMOUS73_events
    data::Ptr{Cvoid}
end

struct wlr_event_pointer_motion
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    delta_x::Cdouble
    delta_y::Cdouble
    unaccel_dx::Cdouble
    unaccel_dy::Cdouble
end

struct wlr_event_pointer_motion_absolute
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    x::Cdouble
    y::Cdouble
end

struct wlr_event_pointer_button
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    button::UInt32
    state::wlr_button_state
end

struct wlr_event_pointer_swipe_begin
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
end

struct wlr_event_pointer_swipe_update
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
    dx::Cdouble
    dy::Cdouble
end

struct wlr_event_pointer_swipe_end
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    cancelled::Bool
end

struct wlr_event_pointer_pinch_begin
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
end

struct wlr_event_pointer_pinch_update
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    fingers::UInt32
    dx::Cdouble
    dy::Cdouble
    scale::Cdouble
    rotation::Cdouble
end

struct wlr_event_pointer_pinch_end
    device::Ptr{wlr_input_device}
    time_msec::UInt32
    cancelled::Bool
end

struct wlr_subcompositor
    _global::Ptr{wl_global}
end

struct ANONYMOUS74_events
    new_surface::wl_signal
    destroy::wl_signal
end

struct wlr_compositor
    _global::Ptr{wl_global}
    renderer::Ptr{wlr_renderer}
    subcompositor::wlr_subcompositor
    display_destroy::wl_listener
    events::ANONYMOUS74_events
end

struct ANONYMOUS75_events
    new_surface::wl_signal
    destroy::wl_signal
end

struct wlr_layer_shell_v1
    _global::Ptr{wl_global}
    display_destroy::wl_listener
    events::ANONYMOUS75_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS76_margin
    top::UInt32
    right::UInt32
    bottom::UInt32
    left::UInt32
end

struct wlr_layer_surface_v1_configure
    link::wl_list
    serial::UInt32
    state::wlr_layer_surface_v1_state
end

struct ANONYMOUS77_events
    destroy::wl_signal
    map::wl_signal
    unmap::wl_signal
    new_popup::wl_signal
end

struct wlr_layer_surface_v1
    surface::Ptr{wlr_surface}
    output::Ptr{wlr_output}
    resource::Ptr{wl_resource}
    shell::Ptr{wlr_layer_shell_v1}
    popups::wl_list
    namespace::Cstring
    added::Bool
    configured::Bool
    mapped::Bool
    closed::Bool
    configure_serial::UInt32
    configure_next_serial::UInt32
    configure_list::wl_list
    acked_configure::Ptr{wlr_layer_surface_v1_configure}
    client_pending::wlr_layer_surface_v1_state
    server_pending::wlr_layer_surface_v1_state
    current::wlr_layer_surface_v1_state
    surface_destroy::wl_listener
    events::ANONYMOUS77_events
    data::Ptr{Cvoid}
end

const WLR_TABLET_V2_TOOL_BUTTONS_CAP = 16

struct ANONYMOUS91_events
    button::wl_signal
    ring::wl_signal
    strip::wl_signal
    attach_tablet::wl_signal
end

struct wlr_tablet_pad
    impl::Ptr{wlr_tablet_pad_impl}
    events::ANONYMOUS91_events
    button_count::Csize_t
    ring_count::Csize_t
    strip_count::Csize_t
    groups::wl_list
    paths::wlr_list
    data::Ptr{Cvoid}
end

const wlr_tablet_pad_client_v2 = Cvoid

struct ANONYMOUS80_events
    button_feedback::wl_signal
    strip_feedback::wl_signal
    ring_feedback::wl_signal
end

struct wlr_tablet_v2_tablet_pad
    link::wl_list
    wlr_pad::Ptr{wlr_tablet_pad}
    wlr_device::Ptr{wlr_input_device}
    clients::wl_list
    group_count::Csize_t
    groups::Ptr{UInt32}
    pad_destroy::wl_listener
    current_client::Ptr{wlr_tablet_pad_client_v2}
    grab::Ptr{wlr_tablet_pad_v2_grab}
    default_grab::wlr_tablet_pad_v2_grab
    events::ANONYMOUS80_events
end

struct wlr_tablet_pad_v2_grab
    interface::Ptr{wlr_tablet_pad_v2_grab_interface}
    pad::Ptr{wlr_tablet_v2_tablet_pad}
    data::Ptr{Cvoid}
end

const wlr_tablet_tool_client_v2 = Cvoid

struct ANONYMOUS79_events
    set_cursor::wl_signal
end

struct wlr_tablet_v2_tablet_tool
    link::wl_list
    wlr_tool::Ptr{wlr_tablet_tool}
    clients::wl_list
    tool_destroy::wl_listener
    current_client::Ptr{wlr_tablet_tool_client_v2}
    focused_surface::Ptr{wlr_surface}
    surface_destroy::wl_listener
    grab::Ptr{wlr_tablet_tool_v2_grab}
    default_grab::wlr_tablet_tool_v2_grab
    proximity_serial::UInt32
    is_down::Bool
    down_serial::UInt32
    num_buttons::Csize_t
    pressed_buttons::NTuple{16, UInt32}
    pressed_serials::NTuple{16, UInt32}
    events::ANONYMOUS79_events
end

struct wlr_tablet_tool_v2_grab
    interface::Ptr{wlr_tablet_tool_v2_grab_interface}
    tool::Ptr{wlr_tablet_v2_tablet_tool}
    data::Ptr{Cvoid}
end

const wlr_tablet_client_v2 = Cvoid

struct ANONYMOUS78_events
    destroy::wl_signal
end

struct wlr_tablet_manager_v2
    wl_global::Ptr{wl_global}
    clients::wl_list
    seats::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS78_events
    data::Ptr{Cvoid}
end

struct wlr_tablet_v2_tablet
    link::wl_list
    wlr_tablet::Ptr{wlr_tablet}
    wlr_device::Ptr{wlr_input_device}
    clients::wl_list
    tool_destroy::wl_listener
    current_client::Ptr{wlr_tablet_client_v2}
end

struct wlr_tablet_v2_event_cursor
    surface::Ptr{wlr_surface}
    serial::UInt32
    hotspot_x::Int32
    hotspot_y::Int32
    seat_client::Ptr{wlr_seat_client}
end

struct wlr_tablet_v2_event_feedback
    description::Cstring
    index::Csize_t
    serial::UInt32
end

struct ANONYMOUS81_events
    destroy::wl_signal
end

struct wlr_linux_dmabuf_v1
    _global::Ptr{wl_global}
    renderer::Ptr{wlr_renderer}
    events::ANONYMOUS81_events
    display_destroy::wl_listener
    renderer_destroy::wl_listener
end

struct ANONYMOUS83_events
    activate::wl_signal
    deactivate::wl_signal
    destroy::wl_signal
end

struct wlr_input_inhibit_manager
    _global::Ptr{wl_global}
    active_client::Ptr{wl_client}
    active_inhibitor::Ptr{wl_resource}
    display_destroy::wl_listener
    events::ANONYMOUS83_events
    data::Ptr{Cvoid}
end

@cenum wlr_text_input_v3_features::UInt32 begin
    WLR_TEXT_INPUT_V3_FEATURE_SURROUNDING_TEXT = 1
    WLR_TEXT_INPUT_v3_FEATURE_CONTENT_TYPE = 2
    WLR_TEXT_INPUT_V3_FEATURE_CURSOR_RECTANGLE = 4
end


struct ANONYMOUS84_surrounding
    text::Cstring
    cursor::UInt32
    anchor::UInt32
end

struct ANONYMOUS85_content_type
    hint::UInt32
    purpose::UInt32
end

struct ANONYMOUS86_cursor_rectangle
    x::Int32
    y::Int32
    width::Int32
    height::Int32
end

struct wlr_text_input_v3_state
    surrounding::ANONYMOUS84_surrounding
    text_change_cause::UInt32
    content_type::ANONYMOUS85_content_type
    cursor_rectangle::ANONYMOUS86_cursor_rectangle
    features::UInt32
end

struct ANONYMOUS87_events
    enable::wl_signal
    commit::wl_signal
    disable::wl_signal
    destroy::wl_signal
end

struct wlr_text_input_v3
    seat::Ptr{wlr_seat}
    resource::Ptr{wl_resource}
    focused_surface::Ptr{wlr_surface}
    pending::wlr_text_input_v3_state
    current::wlr_text_input_v3_state
    current_serial::UInt32
    pending_enabled::Bool
    current_enabled::Bool
    active_features::UInt32
    link::wl_list
    surface_destroy::wl_listener
    seat_destroy::wl_listener
    events::ANONYMOUS87_events
end

struct ANONYMOUS88_events
    text_input::wl_signal
    destroy::wl_signal
end

struct wlr_text_input_manager_v3
    _global::Ptr{wl_global}
    text_inputs::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS88_events
end

struct ANONYMOUS89_events
    destroy::wl_signal
end

struct wlr_pointer_gestures_v1
    _global::Ptr{wl_global}
    swipes::wl_list
    pinches::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS89_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS90_events
    set_mode::wl_signal
    destroy::wl_signal
end

struct wlr_output_power_manager_v1
    _global::Ptr{wl_global}
    output_powers::wl_list
    display_destroy::wl_listener
    events::ANONYMOUS90_events
    data::Ptr{Cvoid}
end

struct wlr_output_power_v1
    resource::Ptr{wl_resource}
    output::Ptr{wlr_output}
    manager::Ptr{wlr_output_power_manager_v1}
    link::wl_list
    output_destroy_listener::wl_listener
    output_enable_listener::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_tablet_pad_group
    link::wl_list
    button_count::Csize_t
    buttons::Ptr{UInt32}
    strip_count::Csize_t
    strips::Ptr{UInt32}
    ring_count::Csize_t
    rings::Ptr{UInt32}
    mode_count::UInt32
end

struct wlr_event_tablet_pad_button
    time_msec::UInt32
    button::UInt32
    state::wlr_button_state
    mode::UInt32
    group::UInt32
end

@cenum wlr_tablet_pad_ring_source::UInt32 begin
    WLR_TABLET_PAD_RING_SOURCE_UNKNOWN = 0
    WLR_TABLET_PAD_RING_SOURCE_FINGER = 1
end


struct wlr_event_tablet_pad_ring
    time_msec::UInt32
    source::wlr_tablet_pad_ring_source
    ring::UInt32
    position::Cdouble
    mode::UInt32
end

@cenum wlr_tablet_pad_strip_source::UInt32 begin
    WLR_TABLET_PAD_STRIP_SOURCE_UNKNOWN = 0
    WLR_TABLET_PAD_STRIP_SOURCE_FINGER = 1
end


struct wlr_event_tablet_pad_strip
    time_msec::UInt32
    source::wlr_tablet_pad_strip_source
    strip::UInt32
    position::Cdouble
    mode::UInt32
end

@cenum wlr_output_state_field::UInt32 begin
    WLR_OUTPUT_STATE_BUFFER = 1
    WLR_OUTPUT_STATE_DAMAGE = 2
    WLR_OUTPUT_STATE_MODE = 4
    WLR_OUTPUT_STATE_ENABLED = 8
    WLR_OUTPUT_STATE_SCALE = 16
    WLR_OUTPUT_STATE_TRANSFORM = 32
    WLR_OUTPUT_STATE_ADAPTIVE_SYNC_ENABLED = 64
    WLR_OUTPUT_STATE_GAMMA_LUT = 128
end


struct wlr_output_event_damage
    output::Ptr{wlr_output}
    damage::Ptr{pixman_region32_t}
end

struct wlr_output_event_precommit
    output::Ptr{wlr_output}
    when::Ptr{timespec}
end

@cenum wlr_output_present_flag::UInt32 begin
    WLR_OUTPUT_PRESENT_VSYNC = 1
    WLR_OUTPUT_PRESENT_HW_CLOCK = 2
    WLR_OUTPUT_PRESENT_HW_COMPLETION = 4
    WLR_OUTPUT_PRESENT_ZERO_COPY = 8
end


struct ANONYMOUS95_events
    activity_notify::wl_signal
    destroy::wl_signal
end

struct wlr_idle
    _global::Ptr{wl_global}
    idle_timers::wl_list
    event_loop::Ptr{wl_event_loop}
    enabled::Bool
    display_destroy::wl_listener
    events::ANONYMOUS95_events
    data::Ptr{Cvoid}
end

struct ANONYMOUS96_events
    idle::wl_signal
    resume::wl_signal
    destroy::wl_signal
end

struct wlr_idle_timeout
    resource::Ptr{wl_resource}
    link::wl_list
    seat::Ptr{wlr_seat}
    idle_source::Ptr{wl_event_source}
    idle_state::Bool
    enabled::Bool
    timeout::UInt32
    events::ANONYMOUS96_events
    input_listener::wl_listener
    seat_destroy::wl_listener
    data::Ptr{Cvoid}
end

const WLR_HAS_EGLMESAEXT_H = 1
const WLR_HAS_SYSTEMD = 1
const WLR_HAS_ELOGIND = 0
const WLR_HAS_X11_BACKEND = 1
const WLR_HAS_XWAYLAND = 1
const WLR_HAS_XCB_ERRORS = 1
const WLR_HAS_XCB_ICCCM = 1
const wlr_xwm = Cvoid
const wlr_xwayland_cursor = Cvoid

struct ANONYMOUS97_events
    ready::wl_signal
    destroy::wl_signal
end

struct wlr_xwayland_server
    pid::pid_t
    client::Ptr{wl_client}
    sigusr1_source::Ptr{wl_event_source}
    wm_fd::NTuple{2, Cint}
    wl_fd::NTuple{2, Cint}
    server_start::Ctime_t
    display::Cint
    display_name::NTuple{16, UInt8}
    x_fd::NTuple{2, Cint}
    x_fd_read_event::NTuple{2, Ptr{wl_event_source}}
    lazy::Bool
    enable_wm::Bool
    wl_display::Ptr{wl_display}
    events::ANONYMOUS97_events
    client_destroy::wl_listener
    display_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_xwayland_server_options
    lazy::Bool
    enable_wm::Bool
end

struct wlr_xwayland_server_ready_event
    server::Ptr{wlr_xwayland_server}
    wm_fd::Cint
end

struct ANONYMOUS98_events
    ready::wl_signal
    new_surface::wl_signal
end

struct wlr_xwayland
    server::Ptr{wlr_xwayland_server}
    xwm::Ptr{wlr_xwm}
    cursor::Ptr{wlr_xwayland_cursor}
    display_name::Cstring
    wl_display::Ptr{wl_display}
    compositor::Ptr{wlr_compositor}
    seat::Ptr{wlr_seat}
    events::ANONYMOUS98_events
    user_event_handler::Ptr{Cvoid}
    server_ready::wl_listener
    server_destroy::wl_listener
    client_destroy::wl_listener
    seat_destroy::wl_listener
    data::Ptr{Cvoid}
end

@cenum wlr_xwayland_surface_decorations::UInt32 begin
    WLR_XWAYLAND_SURFACE_DECORATIONS_ALL = 0
    WLR_XWAYLAND_SURFACE_DECORATIONS_NO_BORDER = 1
    WLR_XWAYLAND_SURFACE_DECORATIONS_NO_TITLE = 2
end


struct wlr_xwayland_surface_hints
    flags::UInt32
    input::UInt32
    initial_state::Int32
    icon_pixmap::xcb_pixmap_t
    icon_window::xcb_window_t
    icon_x::Int32
    icon_y::Int32
    icon_mask::xcb_pixmap_t
    window_group::xcb_window_t
end

struct wlr_xwayland_surface_size_hints
    flags::UInt32
    x::Int32
    y::Int32
    width::Int32
    height::Int32
    min_width::Int32
    min_height::Int32
    max_width::Int32
    max_height::Int32
    width_inc::Int32
    height_inc::Int32
    base_width::Int32
    base_height::Int32
    min_aspect_num::Int32
    min_aspect_den::Int32
    max_aspect_num::Int32
    max_aspect_den::Int32
    win_gravity::UInt32
end

struct ANONYMOUS99_events
    destroy::wl_signal
    request_configure::wl_signal
    request_move::wl_signal
    request_resize::wl_signal
    request_maximize::wl_signal
    request_fullscreen::wl_signal
    request_activate::wl_signal
    map::wl_signal
    unmap::wl_signal
    set_title::wl_signal
    set_class::wl_signal
    set_role::wl_signal
    set_parent::wl_signal
    set_pid::wl_signal
    set_window_type::wl_signal
    set_hints::wl_signal
    set_decorations::wl_signal
    set_override_redirect::wl_signal
    ping_timeout::wl_signal
end

struct wlr_xwayland_surface
    window_id::xcb_window_t
    xwm::Ptr{wlr_xwm}
    surface_id::UInt32
    link::wl_list
    unpaired_link::wl_list
    surface::Ptr{wlr_surface}
    x::Int16
    y::Int16
    width::UInt16
    height::UInt16
    saved_width::UInt16
    saved_height::UInt16
    override_redirect::Bool
    mapped::Bool
    title::Cstring
    class::Cstring
    instance::Cstring
    role::Cstring
    pid::pid_t
    has_utf8_title::Bool
    children::wl_list
    parent::Ptr{wlr_xwayland_surface}
    parent_link::wl_list
    window_type::Ptr{xcb_atom_t}
    window_type_len::Csize_t
    protocols::Ptr{xcb_atom_t}
    protocols_len::Csize_t
    decorations::UInt32
    hints::Ptr{wlr_xwayland_surface_hints}
    hints_urgency::UInt32
    size_hints::Ptr{wlr_xwayland_surface_size_hints}
    pinging::Bool
    ping_timer::Ptr{wl_event_source}
    modal::Bool
    fullscreen::Bool
    maximized_vert::Bool
    maximized_horz::Bool
    has_alpha::Bool
    events::ANONYMOUS99_events
    surface_destroy::wl_listener
    data::Ptr{Cvoid}
end

struct wlr_xwayland_surface_configure_event
    surface::Ptr{wlr_xwayland_surface}
    x::Int16
    y::Int16
    width::UInt16
    height::UInt16
    mask::UInt16
end

struct wlr_xwayland_move_event
    surface::Ptr{wlr_xwayland_surface}
end

struct wlr_xwayland_resize_event
    surface::Ptr{wlr_xwayland_surface}
    edges::UInt32
end

const WLR_DMABUF_MAX_PLANES = 4

@cenum wlr_dmabuf_attributes_flags::UInt32 begin
    WLR_DMABUF_ATTRIBUTES_FLAGS_Y_INVERT = 1
    WLR_DMABUF_ATTRIBUTES_FLAGS_INTERLACED = 2
    WLR_DMABUF_ATTRIBUTES_FLAGS_BOTTOM_FIRST = 4
end

@cenum wlr_renderer_read_pixels_flags::UInt32 begin
    WLR_RENDERER_READ_PIXELS_Y_INVERT = 1
end


struct wlr_drm_format
    format::UInt32
    len::Csize_t
    cap::Csize_t
    modifiers::Ptr{UInt64}
end

struct wlr_drm_format_set
    len::Csize_t
    cap::Csize_t
    formats::Ptr{Ptr{wlr_drm_format}}
end

struct wlr_egl_context
    display::EGLDisplay
    context::EGLContext
    draw_surface::EGLSurface
    read_surface::EGLSurface
end

struct ANONYMOUS101_exts
    bind_wayland_display_wl::Bool
    buffer_age_ext::Bool
    image_base_khr::Bool
    image_dma_buf_export_mesa::Bool
    image_dmabuf_import_ext::Bool
    image_dmabuf_import_modifiers_ext::Bool
    swap_buffers_with_damage::Bool
end

struct ANONYMOUS102_procs
    eglGetPlatformDisplayEXT::PFNEGLGETPLATFORMDISPLAYEXTPROC
    eglCreatePlatformWindowSurfaceEXT::PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC
    eglCreateImageKHR::PFNEGLCREATEIMAGEKHRPROC
    eglDestroyImageKHR::PFNEGLDESTROYIMAGEKHRPROC
    eglQueryWaylandBufferWL::PFNEGLQUERYWAYLANDBUFFERWLPROC
    eglBindWaylandDisplayWL::PFNEGLBINDWAYLANDDISPLAYWLPROC
    eglUnbindWaylandDisplayWL::PFNEGLUNBINDWAYLANDDISPLAYWLPROC
    eglSwapBuffersWithDamage::PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC
    eglQueryDmaBufFormatsEXT::PFNEGLQUERYDMABUFFORMATSEXTPROC
    eglQueryDmaBufModifiersEXT::PFNEGLQUERYDMABUFMODIFIERSEXTPROC
    eglExportDMABUFImageQueryMESA::PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC
    eglExportDMABUFImageMESA::PFNEGLEXPORTDMABUFIMAGEMESAPROC
    eglDebugMessageControlKHR::PFNEGLDEBUGMESSAGECONTROLKHRPROC
end

struct wlr_egl
    platform::EGLenum
    display::EGLDisplay
    config::EGLConfig
    context::EGLContext
    exts::ANONYMOUS101_exts
    procs::ANONYMOUS102_procs
    wl_display::Ptr{wl_display}
    dmabuf_formats::wlr_drm_format_set
    external_only_dmabuf_formats::Ptr{Ptr{EGLBoolean}}
end

struct ANONYMOUS103_exts
    bind_wayland_display_wl::Bool
    buffer_age_ext::Bool
    image_base_khr::Bool
    image_dma_buf_export_mesa::Bool
    image_dmabuf_import_ext::Bool
    image_dmabuf_import_modifiers_ext::Bool
    swap_buffers_with_damage::Bool
end

struct ANONYMOUS104_procs
    eglGetPlatformDisplayEXT::PFNEGLGETPLATFORMDISPLAYEXTPROC
    eglCreatePlatformWindowSurfaceEXT::PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC
    eglCreateImageKHR::PFNEGLCREATEIMAGEKHRPROC
    eglDestroyImageKHR::PFNEGLDESTROYIMAGEKHRPROC
    eglQueryWaylandBufferWL::PFNEGLQUERYWAYLANDBUFFERWLPROC
    eglBindWaylandDisplayWL::PFNEGLBINDWAYLANDDISPLAYWLPROC
    eglUnbindWaylandDisplayWL::PFNEGLUNBINDWAYLANDDISPLAYWLPROC
    eglSwapBuffersWithDamage::PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC
    eglQueryDmaBufFormatsEXT::PFNEGLQUERYDMABUFFORMATSEXTPROC
    eglQueryDmaBufModifiersEXT::PFNEGLQUERYDMABUFMODIFIERSEXTPROC
    eglExportDMABUFImageQueryMESA::PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC
    eglExportDMABUFImageMESA::PFNEGLEXPORTDMABUFIMAGEMESAPROC
    eglDebugMessageControlKHR::PFNEGLDEBUGMESSAGECONTROLKHRPROC
end

struct wlr_gles2_texture_attribs
    target::GLenum
    tex::GLuint
    inverted_y::Bool
    has_alpha::Bool
end

const WLR_VERSION_STR = "0.11.0"
const WLR_VERSION_MAJOR = 0
const WLR_VERSION_MINOR = 11
const WLR_VERSION_MICRO = 0

# Skipping MacroDefinition: WLR_VERSION_NUM ( ( WLR_VERSION_MAJOR << 16 ) | ( WLR_VERSION_MINOR << 8 ) | WLR_VERSION_MICRO )
# Skipping MacroDefinition: wlr_log ( verb , fmt , ... ) _wlr_log ( verb , "[%s:%d] " fmt , _WLR_FILENAME , __LINE__ , ## __VA_ARGS__ )
# Skipping MacroDefinition: wlr_vlog ( verb , fmt , args ) _wlr_vlog ( verb , "[%s:%d] " fmt , _WLR_FILENAME , __LINE__ , args )
# Skipping MacroDefinition: wlr_log_errno ( verb , fmt , ... ) wlr_log ( verb , fmt ": %s" , ## __VA_ARGS__ , strerror ( errno ) )

@cenum wlr_log_importance::UInt32 begin
    WLR_SILENT = 0
    WLR_ERROR = 1
    WLR_INFO = 2
    WLR_DEBUG = 3
    WLR_LOG_IMPORTANCE_LAST = 4
end


const wlr_log_func_t = Ptr{Cvoid}

@cenum wlr_edges::UInt32 begin
    WLR_EDGE_NONE = 0
    WLR_EDGE_TOP = 1
    WLR_EDGE_BOTTOM = 2
    WLR_EDGE_LEFT = 4
    WLR_EDGE_RIGHT = 8
end

                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          wlr_xwayland_server_destroy(server)
    ccall((:wlr_xwayland_server_destroy, xwayland), Cvoid, (Ptr{wlr_xwayland_server},), server)
end

function wlr_xwayland_create(wl_display, compositor, lazy)
    ccall((:wlr_xwayland_create, xwayland), Ptr{wlr_xwayland}, (Ptr{wl_display}, Ptr{wlr_compositor}, Bool), wl_display, compositor, lazy)
end

function wlr_xwayland_destroy(wlr_xwayland)
    ccall((:wlr_xwayland_destroy, xwayland), Cvoid, (Ptr{wlr_xwayland},), wlr_xwayland)
end

function wlr_xwayland_set_cursor(wlr_xwayland, pixels, stride, width, height, hotspot_x, hotspot_y)
    ccall((:wlr_xwayland_set_cursor, xwayland), Cvoid, (Ptr{wlr_xwayland}, Ptr{UInt8}, UInt32, UInt32, UInt32, Int32, Int32), wlr_xwayland, pixels, stride, width, height, hotspot_x, hotspot_y)
end

function wlr_xwayland_surface_activate(surface, activated)
    ccall((:wlr_xwayland_surface_activate, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Bool), surface, activated)
end

function wlr_xwayland_surface_configure(surface, x, y, width, height)
    ccall((:wlr_xwayland_surface_configure, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Int16, Int16, UInt16, UInt16), surface, x, y, width, height)
end

function wlr_xwayland_surface_close(surface)
    ccall((:wlr_xwayland_surface_close, xwayland), Cvoid, (Ptr{wlr_xwayland_surface},), surface)
end

function wlr_xwayland_surface_set_maximized(surface, maximized)
    ccall((:wlr_xwayland_surface_set_maximized, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Bool), surface, maximized)
end

function wlr_xwayland_surface_set_fullscreen(surface, fullscreen)
    ccall((:wlr_xwayland_surface_set_fullscreen, xwayland), Cvoid, (Ptr{wlr_xwayland_surface}, Bool), surface, fullscreen)
end

function wlr_xwayland_set_seat(xwayland, seat)
    ccall((:wlr_xwayland_set_seat, xwayland), Cvoid, (Ptr{wlr_xwayland}, Ptr{wlr_seat}), xwayland, seat)
end

function wlr_surface_is_xwayland_surface(surface)
    ccall((:wlr_surface_is_xwayland_surface, xwayland), Bool, (Ptr{wlr_surface},), surface)
end

function wlr_xwayland_surface_from_wlr_surface(surface)
    ccall((:wlr_xwayland_surface_from_wlr_surface, xwayland), Ptr{wlr_xwayland_surface}, (Ptr{wlr_surface},), surface)
end

function wlr_xwayland_surface_ping(surface)
    ccall((:wlr_xwayland_surface_ping, xwayland), Cvoid, (Ptr{wlr_xwayland_surface},), surface)
end

function wlr_xwayland_or_surface_wants_focus(surface)
    ccall((:wlr_xwayland_or_surface_wants_focus, xwayland), Bool, (Ptr{wlr_xwayland_surface},), surface)
end
# Julia wrapper for header: xcursor.h
# Automatically generated using Clang.jl


function wlr_xcursor_theme_load(name, size)
    ccall((:wlr_xcursor_theme_load, xcursor), Ptr{wlr_xcursor_theme}, (Cstring, Cint), name, size)
end

function wlr_xcursor_theme_destroy(theme)
    ccall((:wlr_xcursor_theme_destroy, xcursor), Cvoid, (Ptr{wlr_xcursor_theme},), theme)
end

function wlr_xcursor_theme_get_cursor(theme, name)
    ccall((:wlr_xcursor_theme_get_cursor, xcursor), Ptr{wlr_xcursor}, (Ptr{wlr_xcursor_theme}, Cstring), theme, name)
end

function wlr_xcursor_frame(cursor, time)
    ccall((:wlr_xcursor_frame, xcursor), Cint, (Ptr{wlr_xcursor}, UInt32), cursor, time)
end

function wlr_xcursor_get_resize_name(edges)
    ccall((:wlr_xcursor_get_resize_name, xcursor), Cstring, (wlr_edges,), edges)
end
# Julia wrapper for header: dmabuf.h
# Automatically generated using Clang.jl


function wlr_dmabuf_attributes_finish(attribs)
    ccall((:wlr_dmabuf_attributes_finish, dmabuf), Cvoid, (Ptr{wlr_dmabuf_attributes},), attribs)
end

function wlr_dmabuf_attributes_copy(dst, src)
    ccall((:wlr_dmabuf_attributes_copy, dmabuf), Bool, (Ptr{wlr_dmabuf_attributes}, Ptr{wlr_dmabuf_attributes}), dst, src)
end
# Julia wrapper for header: interface.h
# Automatically generated using Clang.jl


function wlr_renderer_init(renderer, impl)
    ccall((:wlr_renderer_init, interface), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_renderer_impl}), renderer, impl)
end

function wlr_texture_init(texture, impl, width, height)
    ccall((:wlr_texture_init, interface), Cvoid, (Ptr{wlr_texture}, Ptr{wlr_texture_impl}, UInt32, UInt32), texture, impl, width, height)
end
# Julia wrapper for header: wlr_renderer.h
# Automatically generated using Clang.jl


function wlr_renderer_autocreate(egl, platform, remote_display, config_attribs, visual_id)
    ccall((:wlr_renderer_autocreate, wlr_renderer), Ptr{wlr_renderer}, (Ptr{wlr_egl}, EGLenum, Ptr{Cvoid}, Ptr{EGLint}, EGLint), egl, platform, remote_display, config_attribs, visual_id)
end

function wlr_renderer_begin(r, width, height)
    ccall((:wlr_renderer_begin, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Cint, Cint), r, width, height)
end

function wlr_renderer_end(r)
    ccall((:wlr_renderer_end, wlr_renderer), Cvoid, (Ptr{wlr_renderer},), r)
end

function wlr_renderer_clear(r, color)
    ccall((:wlr_renderer_clear, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{Cfloat}), r, color)
end

function wlr_renderer_scissor(r, box)
    ccall((:wlr_renderer_scissor, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_box}), r, box)
end

function wlr_render_texture(r, texture, projection, x, y, alpha)
    ccall((:wlr_render_texture, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_texture}, Ptr{Cfloat}, Cint, Cint, Cfloat), r, texture, projection, x, y, alpha)
end

function wlr_render_texture_with_matrix(r, texture, matrix, alpha)
    ccall((:wlr_render_texture_with_matrix, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_texture}, Ptr{Cfloat}, Cfloat), r, texture, matrix, alpha)
end

function wlr_render_subtexture_with_matrix(r, texture, box, matrix, alpha)
    ccall((:wlr_render_subtexture_with_matrix, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_texture}, Ptr{wlr_fbox}, Ptr{Cfloat}, Cfloat), r, texture, box, matrix, alpha)
end

function wlr_render_rect(r, box, color, projection)
    ccall((:wlr_render_rect, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_box}, Ptr{Cfloat}, Ptr{Cfloat}), r, box, color, projection)
end

function wlr_render_quad_with_matrix(r, color, matrix)
    ccall((:wlr_render_quad_with_matrix, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{Cfloat}, Ptr{Cfloat}), r, color, matrix)
end

function wlr_render_ellipse(r, box, color, projection)
    ccall((:wlr_render_ellipse, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wlr_box}, Ptr{Cfloat}, Ptr{Cfloat}), r, box, color, projection)
end

function wlr_render_ellipse_with_matrix(r, color, matrix)
    ccall((:wlr_render_ellipse_with_matrix, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{Cfloat}, Ptr{Cfloat}), r, color, matrix)
end

function wlr_renderer_get_formats(r, len)
    ccall((:wlr_renderer_get_formats, wlr_renderer), Ptr{wl_shm_format}, (Ptr{wlr_renderer}, Ptr{Csize_t}), r, len)
end

function wlr_renderer_resource_is_wl_drm_buffer(renderer, buffer)
    ccall((:wlr_renderer_resource_is_wl_drm_buffer, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wl_resource}), renderer, buffer)
end

function wlr_renderer_wl_drm_buffer_get_size(renderer, buffer, width, height)
    ccall((:wlr_renderer_wl_drm_buffer_get_size, wlr_renderer), Cvoid, (Ptr{wlr_renderer}, Ptr{wl_resource}, Ptr{Cint}, Ptr{Cint}), renderer, buffer, width, height)
end

function wlr_renderer_get_dmabuf_formats(renderer)
    ccall((:wlr_renderer_get_dmabuf_formats, wlr_renderer), Ptr{wlr_drm_format_set}, (Ptr{wlr_renderer},), renderer)
end

function wlr_renderer_read_pixels(r, fmt, flags, stride, width, height, src_x, src_y, dst_x, dst_y, data)
    ccall((:wlr_renderer_read_pixels, wlr_renderer), Bool, (Ptr{wlr_renderer}, wl_shm_format, Ptr{UInt32}, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, Ptr{Cvoid}), r, fmt, flags, stride, width, height, src_x, src_y, dst_x, dst_y, data)
end

function wlr_renderer_blit_dmabuf(r, dst, src)
    ccall((:wlr_renderer_blit_dmabuf, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wlr_dmabuf_attributes}, Ptr{wlr_dmabuf_attributes}), r, dst, src)
end

function wlr_renderer_format_supported(r, fmt)
    ccall((:wlr_renderer_format_supported, wlr_renderer), Bool, (Ptr{wlr_renderer}, wl_shm_format), r, fmt)
end

function wlr_renderer_init_wl_display(r, wl_display)
    ccall((:wlr_renderer_init_wl_display, wlr_renderer), Bool, (Ptr{wlr_renderer}, Ptr{wl_display}), r, wl_display)
end

function wlr_renderer_destroy(renderer)
    ccall((:wlr_renderer_destroy, wlr_renderer), Cvoid, (Ptr{wlr_renderer},), renderer)
end
# Julia wrapper for header: egl.h
# Automatically generated using Clang.jl


function wlr_egl_init(egl, platform, remote_display, config_attribs, visual_id)
    ccall((:wlr_egl_init, egl), Bool, (Ptr{wlr_egl}, EGLenum, Ptr{Cvoid}, Ptr{EGLint}, EGLint), egl, platform, remote_display, config_attribs, visual_id)
end

function wlr_egl_finish(egl)
    ccall((:wlr_egl_finish, egl), Cvoid, (Ptr{wlr_egl},), egl)
end

function wlr_egl_bind_display(egl, local_display)
    ccall((:wlr_egl_bind_display, egl), Bool, (Ptr{wlr_egl}, Ptr{wl_display}), egl, local_display)
end

function wlr_egl_create_surface(egl, window)
    ccall((:wlr_egl_create_surface, egl), EGLSurface, (Ptr{wlr_egl}, Ptr{Cvoid}), egl, window)
end

function wlr_egl_create_image_from_wl_drm(egl, data, fmt, width, height, inverted_y)
    ccall((:wlr_egl_create_image_from_wl_drm, egl), EGLImageKHR, (Ptr{wlr_egl}, Ptr{wl_resource}, Ptr{EGLint}, Ptr{Cint}, Ptr{Cint}, Ptr{Bool}), egl, data, fmt, width, height, inverted_y)
end

function wlr_egl_create_image_from_dmabuf(egl, attributes, external_only)
    ccall((:wlr_egl_create_image_from_dmabuf, egl), EGLImageKHR, (Ptr{wlr_egl}, Ptr{wlr_dmabuf_attributes}, Ptr{Bool}), egl, attributes, external_only)
end

function wlr_egl_get_dmabuf_formats(egl)
    ccall((:wlr_egl_get_dmabuf_formats, egl), Ptr{wlr_drm_format_set}, (Ptr{wlr_egl},), egl)
end

function wlr_egl_export_image_to_dmabuf(egl, image, width, height, flags, attribs)
    ccall((:wlr_egl_export_image_to_dmabuf, egl), Bool, (Ptr{wlr_egl}, EGLImageKHR, Int32, Int32, UInt32, Ptr{wlr_dmabuf_attributes}), egl, image, width, height, flags, attribs)
end

function wlr_egl_destroy_image(egl, image)
    ccall((:wlr_egl_destroy_image, egl), Bool, (Ptr{wlr_egl}, EGLImageKHR), egl, image)
end

function wlr_egl_make_current(egl, surface, buffer_age)
    ccall((:wlr_egl_make_current, egl), Bool, (Ptr{wlr_egl}, EGLSurface, Ptr{Cint}), egl, surface, buffer_age)
end

function wlr_egl_unset_current(egl)
    ccall((:wlr_egl_unset_current, egl), Bool, (Ptr{wlr_egl},), egl)
end

function wlr_egl_is_current(egl)
    ccall((:wlr_egl_is_current, egl), Bool, (Ptr{wlr_egl},), egl)
end

function wlr_egl_save_context(context)
    ccall((:wlr_egl_save_context, egl), Cvoid, (Ptr{wlr_egl_context},), context)
end

function wlr_egl_restore_context(context)
    ccall((:wlr_egl_restore_context, egl), Bool, (Ptr{wlr_egl_context},), context)
end

function wlr_egl_swap_buffers(egl, surface, damage)
    ccall((:wlr_egl_swap_buffers, egl), Bool, (Ptr{wlr_egl}, EGLSurface, Ptr{pixman_region32_t}), egl, surface, damage)
end

function wlr_egl_destroy_surface(egl, surface)
    ccall((:wlr_egl_destroy_surface, egl), Bool, (Ptr{wlr_egl}, EGLSurface), egl, surface)
end
# Julia wrapper for header: drm_format_set.h
# Automatically generated using Clang.jl


function wlr_drm_format_set_finish(set)
    ccall((:wlr_drm_format_set_finish, drm_format_set), Cvoid, (Ptr{wlr_drm_format_set},), set)
end

function wlr_drm_format_set_get(set, format)
    ccall((:wlr_drm_format_set_get, drm_format_set), Ptr{wlr_drm_format}, (Ptr{wlr_drm_format_set}, UInt32), set, format)
end

function wlr_drm_format_set_has(set, format, modifier)
    ccall((:wlr_drm_format_set_has, drm_format_set), Bool, (Ptr{wlr_drm_format_set}, UInt32, UInt64), set, format, modifier)
end

function wlr_drm_format_set_add(set, format, modifier)
    ccall((:wlr_drm_format_set_add, drm_format_set), Bool, (Ptr{wlr_drm_format_set}, UInt32, UInt64), set, format, modifier)
end
# Julia wrapper for header: wlr_texture.h
# Automatically generated using Clang.jl


function wlr_texture_from_pixels(renderer, wl_fmt, stride, width, height, data)
    ccall((:wlr_texture_from_pixels, wlr_texture), Ptr{wlr_texture}, (Ptr{wlr_renderer}, wl_shm_format, UInt32, UInt32, UInt32, Ptr{Cvoid}), renderer, wl_fmt, stride, width, height, data)
end

function wlr_texture_from_wl_drm(renderer, data)
    ccall((:wlr_texture_from_wl_drm, wlr_texture), Ptr{wlr_texture}, (Ptr{wlr_renderer}, Ptr{wl_resource}), renderer, data)
end

function wlr_texture_from_dmabuf(renderer, attribs)
    ccall((:wlr_texture_from_dmabuf, wlr_texture), Ptr{wlr_texture}, (Ptr{wlr_renderer}, Ptr{wlr_dmabuf_attributes}), renderer, attribs)
end

function wlr_texture_get_size(texture, width, height)
    ccall((:wlr_texture_get_size, wlr_texture), Cvoid, (Ptr{wlr_texture}, Ptr{Cint}, Ptr{Cint}), texture, width, height)
end

function wlr_texture_is_opaque(texture)
    ccall((:wlr_texture_is_opaque, wlr_texture), Bool, (Ptr{wlr_texture},), texture)
end

function wlr_texture_write_pixels(texture, stride, width, height, src_x, src_y, dst_x, dst_y, data)
    ccall((:wlr_texture_write_pixels, wlr_texture), Bool, (Ptr{wlr_texture}, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32, Ptr{Cvoid}), texture, stride, width, height, src_x, src_y, dst_x, dst_y, data)
end

function wlr_texture_to_dmabuf(texture, attribs)
    ccall((:wlr_texture_to_dmabuf, wlr_texture), Bool, (Ptr{wlr_texture}, Ptr{wlr_dmabuf_attributes}), texture, attribs)
end

function wlr_texture_destroy(texture)
    ccall((:wlr_texture_destroy, wlr_texture), Cvoid, (Ptr{wlr_texture},), texture)
end
# Julia wrapper for header: gles2.h
# Automatically generated using Clang.jl


function wlr_gles2_renderer_create(egl)
    ccall((:wlr_gles2_renderer_create, gles2), Ptr{wlr_renderer}, (Ptr{wlr_egl},), egl)
end

function wlr_gles2_renderer_get_egl(renderer)
    ccall((:wlr_gles2_renderer_get_egl, gles2), Ptr{wlr_egl}, (Ptr{wlr_renderer},), renderer)
end

function wlr_gles2_renderer_check_ext(renderer, ext)
    ccall((:wlr_gles2_renderer_check_ext, gles2), Bool, (Ptr{wlr_renderer}, Cstring), renderer, ext)
end

function wlr_gles2_texture_from_pixels(egl, wl_fmt, stride, width, height, data)
    ccall((:wlr_gles2_texture_from_pixels, gles2), Ptr{wlr_texture}, (Ptr{wlr_egl}, wl_shm_format, UInt32, UInt32, UInt32, Ptr{Cvoid}), egl, wl_fmt, stride, width, height, data)
end

function wlr_gles2_texture_from_wl_drm(egl, data)
    ccall((:wlr_gles2_texture_from_wl_drm, gles2), Ptr{wlr_texture}, (Ptr{wlr_egl}, Ptr{wl_resource}), egl, data)
end

function wlr_gles2_texture_from_dmabuf(egl, attribs)
    ccall((:wlr_gles2_texture_from_dmabuf, gles2), Ptr{wlr_texture}, (Ptr{wlr_egl}, Ptr{wlr_dmabuf_attributes}), egl, attribs)
end

function wlr_texture_is_gles2(texture)
    ccall((:wlr_texture_is_gles2, gles2), Bool, (Ptr{wlr_texture},), texture)
end

function wlr_gles2_texture_get_attribs(texture, attribs)
    ccall((:wlr_gles2_texture_get_attribs, gles2), Cvoid, (Ptr{wlr_texture}, Ptr{wlr_gles2_texture_attribs}), texture, attribs)
end
# Julia wrapper for header: version.h
# Automatically generated using Clang.jl

# Julia wrapper for header: region.h
# Automatically generated using Clang.jl


function wlr_region_scale(dst, src, scale)
    ccall((:wlr_region_scale, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cfloat), dst, src, scale)
end

function wlr_region_scale_xy(dst, src, scale_x, scale_y)
    ccall((:wlr_region_scale_xy, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cfloat, Cfloat), dst, src, scale_x, scale_y)
end

function wlr_region_transform(dst, src, transform, width, height)
    ccall((:wlr_region_transform, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, wl_output_transform, Cint, Cint), dst, src, transform, width, height)
end

function wlr_region_expand(dst, src, distance)
    ccall((:wlr_region_expand, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cint), dst, src, distance)
end

function wlr_region_rotated_bounds(dst, src, rotation, ox, oy)
    ccall((:wlr_region_rotated_bounds, region), Cvoid, (Ptr{pixman_region32_t}, Ptr{pixman_region32_t}, Cfloat, Cint, Cint), dst, src, rotation, ox, oy)
end

function wlr_region_confine(region, x1, y1, x2, y2, x2_out, y2_out)
    ccall((:wlr_region_confine, region), Bool, (Ptr{pixman_region32_t}, Cdouble, Cdouble, Cdouble, Cdouble, Ptr{Cdouble}, Ptr{Cdouble}), region, x1, y1, x2, y2, x2_out, y2_out)
end
# Julia wrapper for header: log.h
# Automatically generated using Clang.jl


function wlr_log_init(verbosity, callback)
    ccall((:wlr_log_init, log), Cvoid, (wlr_log_importance, wlr_log_func_t), verbosity, callback)
end

function wlr_log_get_verbosity()
    ccall((:wlr_log_get_verbosity, log), wlr_log_importance, ())
end
# Julia wrapper for header: edges.h
# Automatically generated using Clang.jl

