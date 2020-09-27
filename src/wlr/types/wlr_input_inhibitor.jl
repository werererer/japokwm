# Julia wrapper for header: wlr_input_inhibitor.h
# Automatically generated using Clang.jl


function wlr_input_inhibit_manager_create(display)
    ccall((:wlr_input_inhibit_manager_create, wlr_input_inhibitor), Ptr{wlr_input_inhibit_manager}, (Ptr{wl_display},), display)
end
e}
    display_destroy::wl_listener
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
end
