# Julia wrapper for header: session.h
# Automatically generated using Clang.jl


function wlr_session_create(disp)
    ccall((:wlr_session_create, session), Ptr{wlr_session}, (Ptr{wl_display},), disp)
end

function wlr_session_destroy(session)
    ccall((:wlr_session_destroy, session), Cvoid, (Ptr{wlr_session},), session)
end

function wlr_session_open_file(session, path)
    ccall((:wlr_session_open_file, session), Cint, (Ptr{wlr_session}, Cstring), session, path)
end

function wlr_session_close_file(session, fd)
    ccall((:wlr_session_close_file, session), Cvoid, (Ptr{wlr_session}, Cint), session, fd)
end

function wlr_session_signal_add(session, fd, listener)
    ccall((:wlr_session_signal_add, session), Cvoid, (Ptr{wlr_session}, Cint, Ptr{wl_listener}), session, fd, listener)
end

function wlr_session_change_vt(session, vt)
    ccall((:wlr_session_change_vt, session), Bool, (Ptr{wlr_session}, UInt32), session, vt)
end

function wlr_session_find_gpus(session, ret_len, ret)
    ccall((:wlr_session_find_gpus, session), Csize_t, (Ptr{wlr_session}, Csize_t, Ptr{Cint}), session, ret_len, ret)
end
