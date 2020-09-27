# Automatically generated using Clang.jl


# Skipping MacroDefinition: wl_client_for_each ( client , list ) for ( client = wl_client_from_link ( ( list ) -> next ) ; wl_client_get_link ( client ) != ( list ) ; client = wl_client_from_link ( wl_client_get_link ( client ) -> next ) )
# Skipping MacroDefinition: wl_resource_for_each ( resource , list ) for ( resource = 0 , resource = wl_resource_from_link ( ( list ) -> next ) ; wl_resource_get_link ( resource ) != ( list ) ; resource = wl_resource_from_link ( wl_resource_get_link ( resource ) -> next ) )
# Skipping MacroDefinition: wl_resource_for_each_safe ( resource , tmp , list ) for ( resource = 0 , tmp = 0 , resource = wl_resource_from_link ( ( list ) -> next ) , tmp = wl_resource_from_link ( ( list ) -> next -> next ) ; wl_resource_get_link ( resource ) != ( list ) ; resource = tmp , tmp = wl_resource_from_link ( wl_resource_get_link ( resource ) -> next ) )

using CEnum
const wl_event_loop_fd_func_t = Ptr{Cvoid}
const wl_event_loop_timer_func_t = Ptr{Cvoid}
const wl_event_loop_signal_func_t = Ptr{Cvoid}
const wl_event_loop_idle_func_t = Ptr{Cvoid}
const wl_event_loop = Cvoid
const wl_event_source = Cvoid

struct wl_list
    prev::Ptr{wl_list}
    next::Ptr{wl_list}
end

const wl_notify_func_t = Ptr{Cvoid}

struct wl_listener
    link::wl_list
    notify::wl_notify_func_t
end

const wl_display = Cvoid
const wl_client = Cvoid
const wl_global_bind_func_t = Ptr{Cvoid}
const wl_global = Cvoid
const wl_display_global_filter_func_t = Ptr{Cvoid}

abstract type Abstract_wl_interface end
struct wl_message{wl_interface<:Abstract_wl_interface}
    name::Cstring
    signature::Cstring
    types::Ptr{Ptr{wl_interface}}
end

struct wl_interface <: Abstract_wl_interface
    name::Cstring
    version::Cint
    method_count::Cint
    methods::Ptr{wl_message}
    event_count::Cint
    events::Ptr{wl_message}
end

struct wl_object
    interface::Ptr{wl_interface}
    implementation::Ptr{Cvoid}
    id::UInt32
end

const wl_resource_destroy_func_t = Ptr{Cvoid}

struct wl_signal
    listener_list::wl_list
end

struct wl_resource
    object::wl_object
    destroy::wl_resource_destroy_func_t
    link::wl_list
    destroy_signal::wl_signal
    client::Ptr{wl_client}
    data::Ptr{Cvoid}
end

const wl_client_for_each_resource_iterator_func_t = Ptr{Cvoid}
const wl_shm_buffer = Cvoid
const wl_shm_pool = Cvoid

@cenum wl_protocol_logger_type::UInt32 begin
    WL_PROTOCOL_LOGGER_REQUEST = 0
    WL_PROTOCOL_LOGGER_EVENT = 1
end


struct wl_argument
    s::Cstring
end

struct wl_protocol_logger_message
    resource::Ptr{wl_resource}
    message_opcode::Cint
    message::Ptr{wl_message}
    arguments_count::Cint
    arguments::Ptr{wl_argument}
end

const wl_protocol_logger_func_t = Ptr{Cvoid}
const wl_protocol_logger = Cvoid
const wl_proxy = Cvoid
const wl_event_queue = Cvoid
const WL_DISPLAY_SYNC = 0
const WL_DISPLAY_GET_REGISTRY = 1
const WL_DISPLAY_ERROR_SINCE_VERSION = 1
const WL_DISPLAY_DELETE_ID_SINCE_VERSION = 1
const WL_DISPLAY_SYNC_SINCE_VERSION = 1
const WL_DISPLAY_GET_REGISTRY_SINCE_VERSION = 1
const WL_REGISTRY_BIND = 0
const WL_REGISTRY_GLOBAL_SINCE_VERSION = 1
const WL_REGISTRY_GLOBAL_REMOVE_SINCE_VERSION = 1
const WL_REGISTRY_BIND_SINCE_VERSION = 1
const WL_CALLBACK_DONE_SINCE_VERSION = 1
const WL_COMPOSITOR_CREATE_SURFACE = 0
const WL_COMPOSITOR_CREATE_REGION = 1
const WL_COMPOSITOR_CREATE_SURFACE_SINCE_VERSION = 1
const WL_COMPOSITOR_CREATE_REGION_SINCE_VERSION = 1
const WL_SHM_POOL_CREATE_BUFFER = 0
const WL_SHM_POOL_DESTROY = 1
const WL_SHM_POOL_RESIZE = 2
const WL_SHM_POOL_CREATE_BUFFER_SINCE_VERSION = 1
const WL_SHM_POOL_DESTROY_SINCE_VERSION = 1
const WL_SHM_POOL_RESIZE_SINCE_VERSION = 1
const WL_SHM_CREATE_POOL = 0
const WL_SHM_FORMAT_SINCE_VERSION = 1
const WL_SHM_CREATE_POOL_SINCE_VERSION = 1
const WL_BUFFER_DESTROY = 0
const WL_BUFFER_RELEASE_SINCE_VERSION = 1
const WL_BUFFER_DESTROY_SINCE_VERSION = 1
const WL_DATA_OFFER_ACCEPT = 0
const WL_DATA_OFFER_RECEIVE = 1
const WL_DATA_OFFER_DESTROY = 2
const WL_DATA_OFFER_FINISH = 3
const WL_DATA_OFFER_SET_ACTIONS = 4
const WL_DATA_OFFER_OFFER_SINCE_VERSION = 1
const WL_DATA_OFFER_SOURCE_ACTIONS_SINCE_VERSION = 3
const WL_DATA_OFFER_ACTION_SINCE_VERSION = 3
const WL_DATA_OFFER_ACCEPT_SINCE_VERSION = 1
const WL_DATA_OFFER_RECEIVE_SINCE_VERSION = 1
const WL_DATA_OFFER_DESTROY_SINCE_VERSION = 1
const WL_DATA_OFFER_FINISH_SINCE_VERSION = 3
const WL_DATA_OFFER_SET_ACTIONS_SINCE_VERSION = 3
const WL_DATA_SOURCE_OFFER = 0
const WL_DATA_SOURCE_DESTROY = 1
const WL_DATA_SOURCE_SET_ACTIONS = 2
const WL_DATA_SOURCE_TARGET_SINCE_VERSION = 1
const WL_DATA_SOURCE_SEND_SINCE_VERSION = 1
const WL_DATA_SOURCE_CANCELLED_SINCE_VERSION = 1
const WL_DATA_SOURCE_DND_DROP_PERFORMED_SINCE_VERSION = 3
const WL_DATA_SOURCE_DND_FINISHED_SINCE_VERSION = 3
const WL_DATA_SOURCE_ACTION_SINCE_VERSION = 3
const WL_DATA_SOURCE_OFFER_SINCE_VERSION = 1
const WL_DATA_SOURCE_DESTROY_SINCE_VERSION = 1
const WL_DATA_SOURCE_SET_ACTIONS_SINCE_VERSION = 3
const WL_DATA_DEVICE_START_DRAG = 0
const WL_DATA_DEVICE_SET_SELECTION = 1
const WL_DATA_DEVICE_RELEASE = 2
const WL_DATA_DEVICE_DATA_OFFER_SINCE_VERSION = 1
const WL_DATA_DEVICE_ENTER_SINCE_VERSION = 1
const WL_DATA_DEVICE_LEAVE_SINCE_VERSION = 1
const WL_DATA_DEVICE_MOTION_SINCE_VERSION = 1
const WL_DATA_DEVICE_DROP_SINCE_VERSION = 1
const WL_DATA_DEVICE_SELECTION_SINCE_VERSION = 1
const WL_DATA_DEVICE_START_DRAG_SINCE_VERSION = 1
const WL_DATA_DEVICE_SET_SELECTION_SINCE_VERSION = 1
const WL_DATA_DEVICE_RELEASE_SINCE_VERSION = 2
const WL_DATA_DEVICE_MANAGER_CREATE_DATA_SOURCE = 0
const WL_DATA_DEVICE_MANAGER_GET_DATA_DEVICE = 1
const WL_DATA_DEVICE_MANAGER_CREATE_DATA_SOURCE_SINCE_VERSION = 1
const WL_DATA_DEVICE_MANAGER_GET_DATA_DEVICE_SINCE_VERSION = 1
const WL_SHELL_GET_SHELL_SURFACE = 0
const WL_SHELL_GET_SHELL_SURFACE_SINCE_VERSION = 1
const WL_SHELL_SURFACE_PONG = 0
const WL_SHELL_SURFACE_MOVE = 1
const WL_SHELL_SURFACE_RESIZE = 2
const WL_SHELL_SURFACE_SET_TOPLEVEL = 3
const WL_SHELL_SURFACE_SET_TRANSIENT = 4
const WL_SHELL_SURFACE_SET_FULLSCREEN = 5
const WL_SHELL_SURFACE_SET_POPUP = 6
const WL_SHELL_SURFACE_SET_MAXIMIZED = 7
const WL_SHELL_SURFACE_SET_TITLE = 8
const WL_SHELL_SURFACE_SET_CLASS = 9
const WL_SHELL_SURFACE_PING_SINCE_VERSION = 1
const WL_SHELL_SURFACE_CONFIGURE_SINCE_VERSION = 1
const WL_SHELL_SURFACE_POPUP_DONE_SINCE_VERSION = 1
const WL_SHELL_SURFACE_PONG_SINCE_VERSION = 1
const WL_SHELL_SURFACE_MOVE_SINCE_VERSION = 1
const WL_SHELL_SURFACE_RESIZE_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_TOPLEVEL_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_TRANSIENT_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_FULLSCREEN_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_POPUP_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_MAXIMIZED_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_TITLE_SINCE_VERSION = 1
const WL_SHELL_SURFACE_SET_CLASS_SINCE_VERSION = 1
const WL_SURFACE_DESTROY = 0
const WL_SURFACE_ATTACH = 1
const WL_SURFACE_DAMAGE = 2
const WL_SURFACE_FRAME = 3
const WL_SURFACE_SET_OPAQUE_REGION = 4
const WL_SURFACE_SET_INPUT_REGION = 5
const WL_SURFACE_COMMIT = 6
const WL_SURFACE_SET_BUFFER_TRANSFORM = 7
const WL_SURFACE_SET_BUFFER_SCALE = 8
const WL_SURFACE_DAMAGE_BUFFER = 9
const WL_SURFACE_ENTER_SINCE_VERSION = 1
const WL_SURFACE_LEAVE_SINCE_VERSION = 1
const WL_SURFACE_DESTROY_SINCE_VERSION = 1
const WL_SURFACE_ATTACH_SINCE_VERSION = 1
const WL_SURFACE_DAMAGE_SINCE_VERSION = 1
const WL_SURFACE_FRAME_SINCE_VERSION = 1
const WL_SURFACE_SET_OPAQUE_REGION_SINCE_VERSION = 1
const WL_SURFACE_SET_INPUT_REGION_SINCE_VERSION = 1
const WL_SURFACE_COMMIT_SINCE_VERSION = 1
const WL_SURFACE_SET_BUFFER_TRANSFORM_SINCE_VERSION = 2
const WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION = 3
const WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION = 4
const WL_SEAT_GET_POINTER = 0
const WL_SEAT_GET_KEYBOARD = 1
const WL_SEAT_GET_TOUCH = 2
const WL_SEAT_RELEASE = 3
const WL_SEAT_CAPABILITIES_SINCE_VERSION = 1
const WL_SEAT_NAME_SINCE_VERSION = 2
const WL_SEAT_GET_POINTER_SINCE_VERSION = 1
const WL_SEAT_GET_KEYBOARD_SINCE_VERSION = 1
const WL_SEAT_GET_TOUCH_SINCE_VERSION = 1
const WL_SEAT_RELEASE_SINCE_VERSION = 5
const WL_POINTER_AXIS_SOURCE_WHEEL_TILT_SINCE_VERSION = 6
const WL_POINTER_SET_CURSOR = 0
const WL_POINTER_RELEASE = 1
const WL_POINTER_ENTER_SINCE_VERSION = 1
const WL_POINTER_LEAVE_SINCE_VERSION = 1
const WL_POINTER_MOTION_SINCE_VERSION = 1
const WL_POINTER_BUTTON_SINCE_VERSION = 1
const WL_POINTER_AXIS_SINCE_VERSION = 1
const WL_POINTER_FRAME_SINCE_VERSION = 5
const WL_POINTER_AXIS_SOURCE_SINCE_VERSION = 5
const WL_POINTER_AXIS_STOP_SINCE_VERSION = 5
const WL_POINTER_AXIS_DISCRETE_SINCE_VERSION = 5
const WL_POINTER_SET_CURSOR_SINCE_VERSION = 1
const WL_POINTER_RELEASE_SINCE_VERSION = 3
const WL_KEYBOARD_RELEASE = 0
const WL_KEYBOARD_KEYMAP_SINCE_VERSION = 1
const WL_KEYBOARD_ENTER_SINCE_VERSION = 1
const WL_KEYBOARD_LEAVE_SINCE_VERSION = 1
const WL_KEYBOARD_KEY_SINCE_VERSION = 1
const WL_KEYBOARD_MODIFIERS_SINCE_VERSION = 1
const WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION = 4
const WL_KEYBOARD_RELEASE_SINCE_VERSION = 3
const WL_TOUCH_RELEASE = 0
const WL_TOUCH_DOWN_SINCE_VERSION = 1
const WL_TOUCH_UP_SINCE_VERSION = 1
const WL_TOUCH_MOTION_SINCE_VERSION = 1
const WL_TOUCH_FRAME_SINCE_VERSION = 1
const WL_TOUCH_CANCEL_SINCE_VERSION = 1
const WL_TOUCH_SHAPE_SINCE_VERSION = 6
const WL_TOUCH_ORIENTATION_SINCE_VERSION = 6
const WL_TOUCH_RELEASE_SINCE_VERSION = 3
const WL_OUTPUT_RELEASE = 0
const WL_OUTPUT_GEOMETRY_SINCE_VERSION = 1
const WL_OUTPUT_MODE_SINCE_VERSION = 1
const WL_OUTPUT_DONE_SINCE_VERSION = 2
const WL_OUTPUT_SCALE_SINCE_VERSION = 2
const WL_OUTPUT_RELEASE_SINCE_VERSION = 3
const WL_REGION_DESTROY = 0
const WL_REGION_ADD = 1
const WL_REGION_SUBTRACT = 2
const WL_REGION_DESTROY_SINCE_VERSION = 1
const WL_REGION_ADD_SINCE_VERSION = 1
const WL_REGION_SUBTRACT_SINCE_VERSION = 1
const WL_SUBCOMPOSITOR_DESTROY = 0
const WL_SUBCOMPOSITOR_GET_SUBSURFACE = 1
const WL_SUBCOMPOSITOR_DESTROY_SINCE_VERSION = 1
const WL_SUBCOMPOSITOR_GET_SUBSURFACE_SINCE_VERSION = 1
const WL_SUBSURFACE_DESTROY = 0
const WL_SUBSURFACE_SET_POSITION = 1
const WL_SUBSURFACE_PLACE_ABOVE = 2
const WL_SUBSURFACE_PLACE_BELOW = 3
const WL_SUBSURFACE_SET_SYNC = 4
const WL_SUBSURFACE_SET_DESYNC = 5
const WL_SUBSURFACE_DESTROY_SINCE_VERSION = 1
const WL_SUBSURFACE_SET_POSITION_SINCE_VERSION = 1
const WL_SUBSURFACE_PLACE_ABOVE_SINCE_VERSION = 1
const WL_SUBSURFACE_PLACE_BELOW_SINCE_VERSION = 1
const WL_SUBSURFACE_SET_SYNC_SINCE_VERSION = 1
const WL_SUBSURFACE_SET_DESYNC_SINCE_VERSION = 1
const wl_buffer = Cvoid
const wl_callback = Cvoid
const wl_compositor = Cvoid
const wl_data_device = Cvoid
const wl_data_device_manager = Cvoid
const wl_data_offer = Cvoid
const wl_data_source = Cvoid
const wl_keyboard = Cvoid
const wl_output = Cvoid
const wl_pointer = Cvoid
const wl_region = Cvoid
const wl_registry = Cvoid
const wl_seat = Cvoid
const wl_shell = Cvoid
const wl_shell_surface = Cvoid
const wl_shm = Cvoid
const wl_subcompositor = Cvoid
const wl_subsurface = Cvoid
const wl_surface = Cvoid
const wl_touch = Cvoid

@cenum wl_display_error::UInt32 begin
    WL_DISPLAY_ERROR_INVALID_OBJECT = 0
    WL_DISPLAY_ERROR_INVALID_METHOD = 1
    WL_DISPLAY_ERROR_NO_MEMORY = 2
    WL_DISPLAY_ERROR_IMPLEMENTATION = 3
end


struct wl_display_listener
    error::Ptr{Cvoid}
    delete_id::Ptr{Cvoid}
end

struct wl_registry_listener
    _global::Ptr{Cvoid}
    global_remove::Ptr{Cvoid}
end

struct wl_callback_listener
    done::Ptr{Cvoid}
end

@cenum wl_shm_error::UInt32 begin
    WL_SHM_ERROR_INVALID_FORMAT = 0
    WL_SHM_ERROR_INVALID_STRIDE = 1
    WL_SHM_ERROR_INVALID_FD = 2
end

@cenum wl_shm_format::UInt32 begin
    WL_SHM_FORMAT_ARGB8888 = 0
    WL_SHM_FORMAT_XRGB8888 = 1
    WL_SHM_FORMAT_C8 = 538982467
    WL_SHM_FORMAT_RGB332 = 943867730
    WL_SHM_FORMAT_BGR233 = 944916290
    WL_SHM_FORMAT_XRGB4444 = 842093144
    WL_SHM_FORMAT_XBGR4444 = 842089048
    WL_SHM_FORMAT_RGBX4444 = 842094674
    WL_SHM_FORMAT_BGRX4444 = 842094658
    WL_SHM_FORMAT_ARGB4444 = 842093121
    WL_SHM_FORMAT_ABGR4444 = 842089025
    WL_SHM_FORMAT_RGBA4444 = 842088786
    WL_SHM_FORMAT_BGRA4444 = 842088770
    WL_SHM_FORMAT_XRGB1555 = 892424792
    WL_SHM_FORMAT_XBGR1555 = 892420696
    WL_SHM_FORMAT_RGBX5551 = 892426322
    WL_SHM_FORMAT_BGRX5551 = 892426306
    WL_SHM_FORMAT_ARGB1555 = 892424769
    WL_SHM_FORMAT_ABGR1555 = 892420673
    WL_SHM_FORMAT_RGBA5551 = 892420434
    WL_SHM_FORMAT_BGRA5551 = 892420418
    WL_SHM_FORMAT_RGB565 = 909199186
    WL_SHM_FORMAT_BGR565 = 909199170
    WL_SHM_FORMAT_RGB888 = 875710290
    WL_SHM_FORMAT_BGR888 = 875710274
    WL_SHM_FORMAT_XBGR8888 = 875709016
    WL_SHM_FORMAT_RGBX8888 = 875714642
    WL_SHM_FORMAT_BGRX8888 = 875714626
    WL_SHM_FORMAT_ABGR8888 = 875708993
    WL_SHM_FORMAT_RGBA8888 = 875708754
    WL_SHM_FORMAT_BGRA8888 = 875708738
    WL_SHM_FORMAT_XRGB2101010 = 808669784
    WL_SHM_FORMAT_XBGR2101010 = 808665688
    WL_SHM_FORMAT_RGBX1010102 = 808671314
    WL_SHM_FORMAT_BGRX1010102 = 808671298
    WL_SHM_FORMAT_ARGB2101010 = 808669761
    WL_SHM_FORMAT_ABGR2101010 = 808665665
    WL_SHM_FORMAT_RGBA1010102 = 808665426
    WL_SHM_FORMAT_BGRA1010102 = 808665410
    WL_SHM_FORMAT_YUYV = 1448695129
    WL_SHM_FORMAT_YVYU = 1431918169
    WL_SHM_FORMAT_UYVY = 1498831189
    WL_SHM_FORMAT_VYUY = 1498765654
    WL_SHM_FORMAT_AYUV = 1448433985
    WL_SHM_FORMAT_NV12 = 842094158
    WL_SHM_FORMAT_NV21 = 825382478
    WL_SHM_FORMAT_NV16 = 909203022
    WL_SHM_FORMAT_NV61 = 825644622
    WL_SHM_FORMAT_YUV410 = 961959257
    WL_SHM_FORMAT_YVU410 = 961893977
    WL_SHM_FORMAT_YUV411 = 825316697
    WL_SHM_FORMAT_YVU411 = 825316953
    WL_SHM_FORMAT_YUV420 = 842093913
    WL_SHM_FORMAT_YVU420 = 842094169
    WL_SHM_FORMAT_YUV422 = 909202777
    WL_SHM_FORMAT_YVU422 = 909203033
    WL_SHM_FORMAT_YUV444 = 875713881
    WL_SHM_FORMAT_YVU444 = 875714137
    WL_SHM_FORMAT_R8 = 538982482
    WL_SHM_FORMAT_R16 = 540422482
    WL_SHM_FORMAT_RG88 = 943212370
    WL_SHM_FORMAT_GR88 = 943215175
    WL_SHM_FORMAT_RG1616 = 842221394
    WL_SHM_FORMAT_GR1616 = 842224199
    WL_SHM_FORMAT_XRGB16161616F = 1211388504
    WL_SHM_FORMAT_XBGR16161616F = 1211384408
    WL_SHM_FORMAT_ARGB16161616F = 1211388481
    WL_SHM_FORMAT_ABGR16161616F = 1211384385
    WL_SHM_FORMAT_XYUV8888 = 1448434008
    WL_SHM_FORMAT_VUY888 = 875713878
    WL_SHM_FORMAT_VUY101010 = 808670550
    WL_SHM_FORMAT_Y210 = 808530521
    WL_SHM_FORMAT_Y212 = 842084953
    WL_SHM_FORMAT_Y216 = 909193817
    WL_SHM_FORMAT_Y410 = 808531033
    WL_SHM_FORMAT_Y412 = 842085465
    WL_SHM_FORMAT_Y416 = 909194329
    WL_SHM_FORMAT_XVYU2101010 = 808670808
    WL_SHM_FORMAT_XVYU12_16161616 = 909334104
    WL_SHM_FORMAT_XVYU16161616 = 942954072
    WL_SHM_FORMAT_Y0L0 = 810299481
    WL_SHM_FORMAT_X0L0 = 810299480
    WL_SHM_FORMAT_Y0L2 = 843853913
    WL_SHM_FORMAT_X0L2 = 843853912
    WL_SHM_FORMAT_YUV420_8BIT = 942691673
    WL_SHM_FORMAT_YUV420_10BIT = 808539481
    WL_SHM_FORMAT_XRGB8888_A8 = 943805016
    WL_SHM_FORMAT_XBGR8888_A8 = 943800920
    WL_SHM_FORMAT_RGBX8888_A8 = 943806546
    WL_SHM_FORMAT_BGRX8888_A8 = 943806530
    WL_SHM_FORMAT_RGB888_A8 = 943798354
    WL_SHM_FORMAT_BGR888_A8 = 943798338
    WL_SHM_FORMAT_RGB565_A8 = 943797586
    WL_SHM_FORMAT_BGR565_A8 = 943797570
    WL_SHM_FORMAT_NV24 = 875714126
    WL_SHM_FORMAT_NV42 = 842290766
    WL_SHM_FORMAT_P210 = 808530512
    WL_SHM_FORMAT_P010 = 808530000
    WL_SHM_FORMAT_P012 = 842084432
    WL_SHM_FORMAT_P016 = 909193296
end


struct wl_shm_listener
    format::Ptr{Cvoid}
end

struct wl_buffer_listener
    release::Ptr{Cvoid}
end

@cenum wl_data_offer_error::UInt32 begin
    WL_DATA_OFFER_ERROR_INVALID_FINISH = 0
    WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK = 1
    WL_DATA_OFFER_ERROR_INVALID_ACTION = 2
    WL_DATA_OFFER_ERROR_INVALID_OFFER = 3
end


struct wl_data_offer_listener
    offer::Ptr{Cvoid}
    source_actions::Ptr{Cvoid}
    action::Ptr{Cvoid}
end

@cenum wl_data_source_error::UInt32 begin
    WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK = 0
    WL_DATA_SOURCE_ERROR_INVALID_SOURCE = 1
end


struct wl_data_source_listener
    target::Ptr{Cvoid}
    send::Ptr{Cvoid}
    cancelled::Ptr{Cvoid}
    dnd_drop_performed::Ptr{Cvoid}
    dnd_finished::Ptr{Cvoid}
    action::Ptr{Cvoid}
end

@cenum wl_data_device_error::UInt32 begin
    WL_DATA_DEVICE_ERROR_ROLE = 0
end


struct wl_data_device_listener
    data_offer::Ptr{Cvoid}
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
    motion::Ptr{Cvoid}
    drop::Ptr{Cvoid}
    selection::Ptr{Cvoid}
end

@cenum wl_data_device_manager_dnd_action::UInt32 begin
    WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE = 0
    WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY = 1
    WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE = 2
    WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK = 4
end

@cenum wl_shell_error::UInt32 begin
    WL_SHELL_ERROR_ROLE = 0
end

@cenum wl_shell_surface_resize::UInt32 begin
    WL_SHELL_SURFACE_RESIZE_NONE = 0
    WL_SHELL_SURFACE_RESIZE_TOP = 1
    WL_SHELL_SURFACE_RESIZE_BOTTOM = 2
    WL_SHELL_SURFACE_RESIZE_LEFT = 4
    WL_SHELL_SURFACE_RESIZE_TOP_LEFT = 5
    WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT = 6
    WL_SHELL_SURFACE_RESIZE_RIGHT = 8
    WL_SHELL_SURFACE_RESIZE_TOP_RIGHT = 9
    WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT = 10
end

@cenum wl_shell_surface_transient::UInt32 begin
    WL_SHELL_SURFACE_TRANSIENT_INACTIVE = 1
end

@cenum wl_shell_surface_fullscreen_method::UInt32 begin
    WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT = 0
    WL_SHELL_SURFACE_FULLSCREEN_METHOD_SCALE = 1
    WL_SHELL_SURFACE_FULLSCREEN_METHOD_DRIVER = 2
    WL_SHELL_SURFACE_FULLSCREEN_METHOD_FILL = 3
end


struct wl_shell_surface_listener
    ping::Ptr{Cvoid}
    configure::Ptr{Cvoid}
    popup_done::Ptr{Cvoid}
end

@cenum wl_surface_error::UInt32 begin
    WL_SURFACE_ERROR_INVALID_SCALE = 0
    WL_SURFACE_ERROR_INVALID_TRANSFORM = 1
end


struct wl_surface_listener
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
end

@cenum wl_seat_capability::UInt32 begin
    WL_SEAT_CAPABILITY_POINTER = 1
    WL_SEAT_CAPABILITY_KEYBOARD = 2
    WL_SEAT_CAPABILITY_TOUCH = 4
end


struct wl_seat_listener
    capabilities::Ptr{Cvoid}
    name::Ptr{Cvoid}
end

@cenum wl_pointer_error::UInt32 begin
    WL_POINTER_ERROR_ROLE = 0
end

@cenum wl_pointer_button_state::UInt32 begin
    WL_POINTER_BUTTON_STATE_RELEASED = 0
    WL_POINTER_BUTTON_STATE_PRESSED = 1
end

@cenum wl_pointer_axis::UInt32 begin
    WL_POINTER_AXIS_VERTICAL_SCROLL = 0
    WL_POINTER_AXIS_HORIZONTAL_SCROLL = 1
end

@cenum wl_pointer_axis_source::UInt32 begin
    WL_POINTER_AXIS_SOURCE_WHEEL = 0
    WL_POINTER_AXIS_SOURCE_FINGER = 1
    WL_POINTER_AXIS_SOURCE_CONTINUOUS = 2
    WL_POINTER_AXIS_SOURCE_WHEEL_TILT = 3
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

@cenum wl_keyboard_keymap_format::UInt32 begin
    WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP = 0
    WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 = 1
end

@cenum wl_keyboard_key_state::UInt32 begin
    WL_KEYBOARD_KEY_STATE_RELEASED = 0
    WL_KEYBOARD_KEY_STATE_PRESSED = 1
end


struct wl_keyboard_listener
    keymap::Ptr{Cvoid}
    enter::Ptr{Cvoid}
    leave::Ptr{Cvoid}
    key::Ptr{Cvoid}
    modifiers::Ptr{Cvoid}
    repeat_info::Ptr{Cvoid}
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

@cenum wl_output_subpixel::UInt32 begin
    WL_OUTPUT_SUBPIXEL_UNKNOWN = 0
    WL_OUTPUT_SUBPIXEL_NONE = 1
    WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB = 2
    WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR = 3
    WL_OUTPUT_SUBPIXEL_VERTICAL_RGB = 4
    WL_OUTPUT_SUBPIXEL_VERTICAL_BGR = 5
end

@cenum wl_output_transform::UInt32 begin
    WL_OUTPUT_TRANSFORM_NORMAL = 0
    WL_OUTPUT_TRANSFORM_90 = 1
    WL_OUTPUT_TRANSFORM_180 = 2
    WL_OUTPUT_TRANSFORM_270 = 3
    WL_OUTPUT_TRANSFORM_FLIPPED = 4
    WL_OUTPUT_TRANSFORM_FLIPPED_90 = 5
    WL_OUTPUT_TRANSFORM_FLIPPED_180 = 6
    WL_OUTPUT_TRANSFORM_FLIPPED_270 = 7
end

@cenum wl_output_mode::UInt32 begin
    WL_OUTPUT_MODE_CURRENT = 1
    WL_OUTPUT_MODE_PREFERRED = 2
end


struct wl_output_listener
    geometry::Ptr{Cvoid}
    mode::Ptr{Cvoid}
    done::Ptr{Cvoid}
    scale::Ptr{Cvoid}
end

@cenum wl_subcompositor_error::UInt32 begin
    WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE = 0
end

@cenum wl_subsurface_error::UInt32 begin
    WL_SUBSURFACE_ERROR_BAD_SURFACE = 0
end


const wl_cursor_theme = Cvoid

struct wl_cursor_image
    width::UInt32
    height::UInt32
    hotspot_x::UInt32
    hotspot_y::UInt32
    delay::UInt32
end

struct wl_cursor
    image_count::UInt32
    images::Ptr{Ptr{wl_cursor_image}}
    name::Cstring
end

const WL_EGL_WINDOW_VERSION = 3

struct wl_egl_window
    version::Ptr{Cint}
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

const WL_EGL_PLATFORM = 1
const WL_DISPLAY_ERROR = 0
const WL_DISPLAY_DELETE_ID = 1
const WL_REGISTRY_GLOBAL = 0
const WL_REGISTRY_GLOBAL_REMOVE = 1
const WL_CALLBACK_DONE = 0
const WL_SHM_FORMAT = 0
const WL_BUFFER_RELEASE = 0
const WL_DATA_OFFER_OFFER = 0
const WL_DATA_OFFER_SOURCE_ACTIONS = 1
const WL_DATA_OFFER_ACTION = 2
const WL_DATA_SOURCE_TARGET = 0
const WL_DATA_SOURCE_SEND = 1
const WL_DATA_SOURCE_CANCELLED = 2
const WL_DATA_SOURCE_DND_DROP_PERFORMED = 3
const WL_DATA_SOURCE_DND_FINISHED = 4
const WL_DATA_SOURCE_ACTION = 5
const WL_DATA_DEVICE_DATA_OFFER = 0
const WL_DATA_DEVICE_ENTER = 1
const WL_DATA_DEVICE_LEAVE = 2
const WL_DATA_DEVICE_MOTION = 3
const WL_DATA_DEVICE_DROP = 4
const WL_DATA_DEVICE_SELECTION = 5
const WL_SHELL_SURFACE_PING = 0
const WL_SHELL_SURFACE_CONFIGURE = 1
const WL_SHELL_SURFACE_POPUP_DONE = 2
const WL_SURFACE_ENTER = 0
const WL_SURFACE_LEAVE = 1
const WL_SEAT_CAPABILITIES = 0
const WL_SEAT_NAME = 1
const WL_POINTER_ENTER = 0
const WL_POINTER_LEAVE = 1
const WL_POINTER_MOTION = 2
const WL_POINTER_BUTTON = 3
const WL_POINTER_AXIS = 4
const WL_POINTER_FRAME = 5
const WL_POINTER_AXIS_SOURCE = 6
const WL_POINTER_AXIS_STOP = 7
const WL_POINTER_AXIS_DISCRETE = 8
const WL_KEYBOARD_KEYMAP = 0
const WL_KEYBOARD_ENTER = 1
const WL_KEYBOARD_LEAVE = 2
const WL_KEYBOARD_KEY = 3
const WL_KEYBOARD_MODIFIERS = 4
const WL_KEYBOARD_REPEAT_INFO = 5
const WL_TOUCH_DOWN = 0
const WL_TOUCH_UP = 1
const WL_TOUCH_MOTION = 2
const WL_TOUCH_FRAME = 3
const WL_TOUCH_CANCEL = 4
const WL_TOUCH_SHAPE = 5
const WL_TOUCH_ORIENTATION = 6
const WL_OUTPUT_GEOMETRY = 0
const WL_OUTPUT_MODE = 1
const WL_OUTPUT_DONE = 2
const WL_OUTPUT_SCALE = 3

struct wl_display_interface
    sync::Ptr{Cvoid}
    get_registry::Ptr{Cvoid}
end

struct wl_registry_interface
    bind::Ptr{Cvoid}
end

struct wl_compositor_interface
    create_surface::Ptr{Cvoid}
    create_region::Ptr{Cvoid}
end

struct wl_shm_pool_interface
    create_buffer::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    resize::Ptr{Cvoid}
end

struct wl_shm_interface
    create_pool::Ptr{Cvoid}
end

struct wl_buffer_interface
    destroy::Ptr{Cvoid}
end

struct wl_data_offer_interface
    accept::Ptr{Cvoid}
    receive::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    finish::Ptr{Cvoid}
    set_actions::Ptr{Cvoid}
end

struct wl_data_source_interface
    offer::Ptr{Cvoid}
    destroy::Ptr{Cvoid}
    set_actions::Ptr{Cvoid}
end

struct wl_data_device_interface
    start_drag::Ptr{Cvoid}
    set_selection::Ptr{Cvoid}
    release::Ptr{Cvoid}
end

struct wl_data_device_manager_interface
    create_data_source::Ptr{Cvoid}
    get_data_device::Ptr{Cvoid}
end

struct wl_shell_interface
    get_shell_surface::Ptr{Cvoid}
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

struct wl_seat_interface
    get_pointer::Ptr{Cvoid}
    get_keyboard::Ptr{Cvoid}
    get_touch::Ptr{Cvoid}
    release::Ptr{Cvoid}
end

struct wl_pointer_interface
    set_cursor::Ptr{Cvoid}
    release::Ptr{Cvoid}
end

struct wl_keyboard_interface
    release::Ptr{Cvoid}
end

struct wl_touch_interface
    release::Ptr{Cvoid}
end

struct wl_output_interface
    release::Ptr{Cvoid}
end

struct wl_region_interface
    destroy::Ptr{Cvoid}
    add::Ptr{Cvoid}
    subtract::Ptr{Cvoid}
end

struct wl_subcompositor_interface
    destroy::Ptr{Cvoid}
    get_subsurface::Ptr{Cvoid}
end

struct wl_subsurface_interface
    destroy::Ptr{Cvoid}
    set_position::Ptr{Cvoid}
    place_above::Ptr{Cvoid}
    place_below::Ptr{Cvoid}
    set_sync::Ptr{Cvoid}
    set_desync::Ptr{Cvoid}
end

# Skipping MacroDefinition: WL_EXPORT __attribute__ ( ( visibility ( "default" ) ) )
# Skipping MacroDefinition: WL_DEPRECATED __attribute__ ( ( deprecated ) )
# Skipping MacroDefinition: WL_PRINTF ( x , y ) __attribute__ ( ( __format__ ( __printf__ , x , y ) ) )
# Skipping MacroDefinition: wl_container_of ( ptr , sample , member ) ( __typeof__ ( sample ) ) ( ( char * ) ( ptr ) - offsetof ( __typeof__ ( * sample ) , member ) )
# Skipping MacroDefinition: wl_list_for_each ( pos , head , member ) for ( pos = wl_container_of ( ( head ) -> next , pos , member ) ; & pos -> member != ( head ) ; pos = wl_container_of ( pos -> member . next , pos , member ) )
# Skipping MacroDefinition: wl_list_for_each_safe ( pos , tmp , head , member ) for ( pos = wl_container_of ( ( head ) -> next , pos , member ) , tmp = wl_container_of ( ( pos ) -> member . next , tmp , member ) ; & pos -> member != ( head ) ; pos = tmp , tmp = wl_container_of ( pos -> member . next , tmp , member ) )
# Skipping MacroDefinition: wl_list_for_each_reverse ( pos , head , member ) for ( pos = wl_container_of ( ( head ) -> prev , pos , member ) ; & pos -> member != ( head ) ; pos = wl_container_of ( pos -> member . prev , pos , member ) )
# Skipping MacroDefinition: wl_list_for_each_reverse_safe ( pos , tmp , head , member ) for ( pos = wl_container_of ( ( head ) -> prev , pos , member ) , tmp = wl_container_of ( ( pos ) -> member . prev , tmp , member ) ; & pos -> member != ( head ) ; pos = tmp , tmp = wl_container_of ( pos -> member . prev , tmp , member ) )
# Skipping MacroDefinition: wl_array_for_each ( pos , array ) for ( pos = ( array ) -> data ; ( const char * ) pos < ( ( const char * ) ( array ) -> data + ( array ) -> size ) ; ( pos ) ++ )

struct wl_array
    size::Csize_t
    alloc::Csize_t
    data::Ptr{Cvoid}
end

const wl_fixed_t = Int32
const wl_dispatcher_func_t = Ptr{Cvoid}
const wl_log_func_t = Ptr{Cvoid}

@cenum wl_iterator_result::UInt32 begin
    WL_ITERATOR_STOP = 0
    WL_ITERATOR_CONTINUE = 1
end


const WAYLAND_VERSION_MAJOR = 1
const WAYLAND_VERSION_MINOR = 18
const WAYLAND_VERSION_MICRO = 0
const WAYLAND_VERSION = "1.18.0"

function wl_output_send_geometry(resource_, x, y, physical_width, physical_height, subpixel, make, model, transform)
    ccall((:wl_output_send_geometry, var"wayland-server-protocol"), Cvoid, (Ptr{wl_resource}, Int32, Int32, Int32, Int32, Int32, Cstring, Cstring, Int32), resource_, x, y, physical_width, physical_height, subpixel, make, model, transform)
end

function wl_output_send_mode(resource_, flags, width, height, refresh)
    ccall((:wl_output_send_mode, var"wayland-server-protocol"), Cvoid, (Ptr{wl_resource}, UInt32, Int32, Int32, Int32), resource_, flags, width, height, refresh)
end

function wl_output_send_done(resource_)
    ccall((:wl_output_send_done, var"wayland-server-protocol"), Cvoid, (Ptr{wl_resource},), resource_)
end

function wl_output_send_scale(resource_, factor)
    ccall((:wl_output_send_scale, var"wayland-server-protocol"), Cvoid, (Ptr{wl_resource}, Int32), resource_, factor)
end
# Julia wrapper for header: wayland-util.h
# Automatically generated using Clang.jl


function wl_list_init(list)
    ccall((:wl_list_init, var"wayland-util"), Cvoid, (Ptr{wl_list},), list)
end

function wl_list_insert(list, elm)
    ccall((:wl_list_insert, var"wayland-util"), Cvoid, (Ptr{wl_list}, Ptr{wl_list}), list, elm)
end

function wl_list_remove(elm)
    ccall((:wl_list_remove, var"wayland-util"), Cvoid, (Ptr{wl_list},), elm)
end

function wl_list_length(list)
    ccall((:wl_list_length, var"wayland-util"), Cint, (Ptr{wl_list},), list)
end

function wl_list_empty(list)
    ccall((:wl_list_empty, var"wayland-util"), Cint, (Ptr{wl_list},), list)
end

function wl_list_insert_list(list, other)
    ccall((:wl_list_insert_list, var"wayland-util"), Cvoid, (Ptr{wl_list}, Ptr{wl_list}), list, other)
end

function wl_array_init(array)
    ccall((:wl_array_init, var"wayland-util"), Cvoid, (Ptr{wl_array},), array)
end

function wl_array_release(array)
    ccall((:wl_array_release, var"wayland-util"), Cvoid, (Ptr{wl_array},), array)
end

function wl_array_add(array, size)
    ccall((:wl_array_add, var"wayland-util"), Ptr{Cvoid}, (Ptr{wl_array}, Csize_t), array, size)
end

function wl_array_copy(array, source)
    ccall((:wl_array_copy, var"wayland-util"), Cint, (Ptr{wl_array}, Ptr{wl_array}), array, source)
end

function wl_fixed_to_double(f)
    ccall((:wl_fixed_to_double, var"wayland-util"), Cdouble, (wl_fixed_t,), f)
end

function wl_fixed_from_double(d)
    ccall((:wl_fixed_from_double, var"wayland-util"), wl_fixed_t, (Cdouble,), d)
end

function wl_fixed_to_int(f)
    ccall((:wl_fixed_to_int, var"wayland-util"), Cint, (wl_fixed_t,), f)
end

function wl_fixed_from_int(i)
    ccall((:wl_fixed_from_int, var"wayland-util"), wl_fixed_t, (Cint,), i)
end
# Julia wrapper for header: wayland-version.h
# Automatically generated using Clang.jl

