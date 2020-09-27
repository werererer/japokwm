# Julia wrapper for header: wlr_text_input_v3.h
# Automatically generated using Clang.jl


function wlr_text_input_manager_v3_create(wl_display)
    ccall((:wlr_text_input_manager_v3_create, wlr_text_input_v3), Ptr{wlr_text_input_manager_v3}, (Ptr{wl_display},), wl_display)
end

function wlr_text_input_v3_send_enter(text_input, wlr_surface)
    ccall((:wlr_text_input_v3_send_enter, wlr_text_input_v3), Cvoid, (Ptr{wlr_text_input_v3}, Ptr{wlr_surface}), text_input, wlr_surface)
end

function wlr_text_input_v3_send_leave(text_input)
    ccall((:wlr_text_input_v3_send_leave, wlr_text_input_v3), Cvoid, (Ptr{wlr_text_input_v3},), text_input)
end

function wlr_text_input_v3_send_preedit_string(text_input, text, cursor_begin, cursor_end)
    ccall((:wlr_text_input_v3_send_preedit_string, wlr_text_input_v3), Cvoid, (Ptr{wlr_text_input_v3}, Cstring, UInt32, UInt32), text_input, text, cursor_begin, cursor_end)
end

function wlr_text_input_v3_send_commit_string(text_input, text)
    ccall((:wlr_text_input_v3_send_commit_string, wlr_text_input_v3), Cvoid, (Ptr{wlr_text_input_v3}, Cstring), text_input, text)
end

function wlr_text_input_v3_send_delete_surrounding_text(text_input, before_length, after_length)
    ccall((:wlr_text_input_v3_send_delete_surrounding_text, wlr_text_input_v3), Cvoid, (Ptr{wlr_text_input_v3}, UInt32, UInt32), text_input, before_length, after_length)
end

function wlr_text_input_v3_send_done(text_input)
    ccall((:wlr_text_input_v3_send_done, wlr_text_input_v3), Cvoid, (Ptr{wlr_text_input_v3},), text_input)
end
