# Julia wrapper for header: wlr_presentation_time.h
# Automatically generated using Clang.jl


function wlr_presentation_create(display, backend)
    ccall((:wlr_presentation_create, wlr_presentation_time), Ptr{wlr_presentation}, (Ptr{wl_display}, Ptr{wlr_backend}), display, backend)
end

function wlr_presentation_surface_sampled(presentation, surface)
    ccall((:wlr_presentation_surface_sampled, wlr_presentation_time), Ptr{wlr_presentation_feedback}, (Ptr{wlr_presentation}, Ptr{wlr_surface}), presentation, surface)
end

function wlr_presentation_feedback_send_presented(feedback, event)
    ccall((:wlr_presentation_feedback_send_presented, wlr_presentation_time), Cvoid, (Ptr{wlr_presentation_feedback}, Ptr{wlr_presentation_event}), feedback, event)
end

function wlr_presentation_feedback_destroy(feedback)
    ccall((:wlr_presentation_feedback_destroy, wlr_presentation_time), Cvoid, (Ptr{wlr_presentation_feedback},), feedback)
end

function wlr_presentation_event_from_output(event, output_event)
    ccall((:wlr_presentation_event_from_output, wlr_presentation_time), Cvoid, (Ptr{wlr_presentation_event}, Ptr{wlr_output_event_present}), event, output_event)
end

function wlr_presentation_surface_sampled_on_output(presentation, surface, output)
    ccall((:wlr_presentation_surface_sampled_on_output, wlr_presentation_time), Cvoid, (Ptr{wlr_presentation}, Ptr{wlr_surface}, Ptr{wlr_output}), presentation, surface, output)
end
