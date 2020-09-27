# Julia wrapper for header: wlr_output.h
# Automatically generated using Clang.jl


function wlr_output_init(output, backend, impl, display)
    ccall((:wlr_output_init, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{wlr_backend}, Ptr{wlr_output_impl}, Ptr{wl_display}), output, backend, impl, display)
end

function wlr_output_update_mode(output, mode)
    ccall((:wlr_output_update_mode, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{wlr_output_mode}), output, mode)
end

function wlr_output_update_custom_mode(output, width, height, refresh)
    ccall((:wlr_output_update_custom_mode, wlr_output), Cvoid, (Ptr{wlr_output}, Int32, Int32, Int32), output, width, height, refresh)
end

function wlr_output_update_enabled(output, enabled)
    ccall((:wlr_output_update_enabled, wlr_output), Cvoid, (Ptr{wlr_output}, Bool), output, enabled)
end

function wlr_output_update_needs_frame(output)
    ccall((:wlr_output_update_needs_frame, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_damage_whole(output)
    ccall((:wlr_output_damage_whole, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_send_frame(output)
    ccall((:wlr_output_send_frame, wlr_output), Cvoid, (Ptr{wlr_output},), output)
end

function wlr_output_send_present(output, event)
    ccall((:wlr_output_send_present, wlr_output), Cvoid, (Ptr{wlr_output}, Ptr{wlr_output_event_present}), output, event)
end
