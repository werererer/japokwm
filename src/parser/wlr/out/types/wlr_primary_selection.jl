# Julia wrapper for header: wlr_primary_selection.h
# Automatically generated using Clang.jl


function wlr_primary_selection_source_init(source, impl)
    ccall((:wlr_primary_selection_source_init, wlr_primary_selection), Cvoid, (Ptr{wlr_primary_selection_source}, Ptr{wlr_primary_selection_source_impl}), source, impl)
end

function wlr_primary_selection_source_destroy(source)
    ccall((:wlr_primary_selection_source_destroy, wlr_primary_selection), Cvoid, (Ptr{wlr_primary_selection_source},), source)
end

function wlr_primary_selection_source_send(source, mime_type, fd)
    ccall((:wlr_primary_selection_source_send, wlr_primary_selection), Cvoid, (Ptr{wlr_primary_selection_source}, Cstring, Cint), source, mime_type, fd)
end

function wlr_seat_request_set_primary_selection(seat, client, source, serial)
    ccall((:wlr_seat_request_set_primary_selection, wlr_primary_selection), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_seat_client}, Ptr{wlr_primary_selection_source}, UInt32), seat, client, source, serial)
end

function wlr_seat_set_primary_selection(seat, source, serial)
    ccall((:wlr_seat_set_primary_selection, wlr_primary_selection), Cvoid, (Ptr{wlr_seat}, Ptr{wlr_primary_selection_source}, UInt32), seat, source, serial)
end
