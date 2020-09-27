# Julia wrapper for header: wlr_data_device.h
# Automatically generated using Clang.jl


function wlr_data_device_manager_create(display)
    ccall((:wlr_data_device_manager_create, wlr_data_device), Ptr{wlr_data_device_manager}, (Ptr{wl_display},), display)
end

function wlr_seat_request_set_selection(seat, client, source, serial)
    ccall((:wlr_seat_request_set_selection, wlr_data_device), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_seat_client}, Ptr{wlr_data_source}, UInt32), seat, client, source, serial)
end

function wlr_seat_set_selection(seat, source, serial)
    ccall((:wlr_seat_set_selection, wlr_data_device), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_data_source}, UInt32), seat, source, serial)
end

function wlr_drag_create(seat_client, source, icon_surface)
    ccall((:wlr_drag_create, wlr_data_device), Ptr{wlr_drag}, (Ptr{wlr_seat_client}, Ptr{wlr_data_source}, Ptr{wlr_surface}), seat_client, source, icon_surface)
end

function wlr_seat_request_start_drag(seat, drag, origin, serial)
    ccall((:wlr_seat_request_start_drag, wlr_data_device), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_drag}, Ptr{wlr_surface}, UInt32), seat, drag, origin, serial)
end

function wlr_seat_start_drag(seat, drag, serial)
    ccall((:wlr_seat_start_drag, wlr_data_device), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_drag}, UInt32), seat, drag, serial)
end

function wlr_seat_start_pointer_drag(seat, drag, serial)
    ccall((:wlr_seat_start_pointer_drag, wlr_data_device), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_drag}, UInt32), seat, drag, serial)
end

function wlr_seat_start_touch_drag(seat, drag, serial, point)
    ccall((:wlr_seat_start_touch_drag, wlr_data_device), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_drag}, UInt32, Ptr{wlr_touch_point}), seat, drag, serial, point)
end

function wlr_data_source_init(source, impl)
    ccall((:wlr_data_source_init, wlr_data_device), Cvoid, (Ptr{wlr_data_source}, Ptr{wlr_data_source_impl}), source, impl)
end

function wlr_data_source_send(source, mime_type, fd)
    ccall((:wlr_data_source_send, wlr_data_device), Cvoid, (Ptr{wlr_data_source}, Cstring, Int32), source, mime_type, fd)
end

function wlr_data_source_accept(source, serial, mime_type)
    ccall((:wlr_data_source_accept, wlr_data_device), Cvoid, (Ptr{wlr_data_source}, UInt32, Cstring), source, serial, mime_type)
end

function wlr_data_source_destroy(source)
    ccall((:wlr_data_source_destroy, wlr_data_device), Cvoid, (Ptr{wlr_data_source},), source)
end

function wlr_data_source_dnd_drop(source)
    ccall((:wlr_data_source_dnd_drop, wlr_data_device), Cvoid, (Ptr{wlr_data_source},), source)
end

function wlr_data_source_dnd_finish(source)
    ccall((:wlr_data_source_dnd_finish, wlr_data_device), Cvoid, (Ptr{wlr_data_source},), source)
end

function wlr_data_source_dnd_action(source, action)
    ccall((:wlr_data_source_dnd_action, wlr_data_device), Cvoid, (Ptr{wlr_data_source}, wl_data_device_manager_dnd_action), source, action)
end
