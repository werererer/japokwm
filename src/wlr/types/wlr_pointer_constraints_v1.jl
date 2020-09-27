# Julia wrapper for header: wlr_pointer_constraints_v1.h
# Automatically generated using Clang.jl


function wlr_pointer_constraints_v1_create(display)
    ccall((:wlr_pointer_constraints_v1_create, wlr_pointer_constraints_v1), Ptr{wlr_pointer_constraints_v1}, (Ptr{wl_display},), display)
end

function wlr_pointer_constraints_v1_constraint_for_surface(pointer_constraints, surface, seat)
    ccall((:wlr_pointer_constraints_v1_constraint_for_surface, wlr_pointer_constraints_v1), Ptr{wlr_pointer_constraint_v1}, (Ptr{wlr_pointer_constraints_v1}, Ptr{wlr_surface}, Ptr{wlr_seat}), pointer_constraints, surface, seat)
end

function wlr_pointer_constraint_v1_send_activated(constraint)
    ccall((:wlr_pointer_constraint_v1_send_activated, wlr_pointer_constraints_v1), Cvoid, (Ptr{wlr_pointer_constraint_v1},), constraint)
end

function wlr_pointer_constraint_v1_send_deactivated(constraint)
    ccall((:wlr_pointer_constraint_v1_send_deactivated, wlr_pointer_constraints_v1), Cvoid, (Ptr{wlr_pointer_constraint_v1},), constraint)
end
ers::wl_list
    pointer_state::wlr_seat_pointer_state
    keyboard_state::wlr_seat_keyboard_state
    touch_state::wlr_seat_touch_state
    display_destroy::wl_listener
    selection_source_destroy::wl_listener
    primary_selection_source_destroy::wl_listener
    drag_source_destroy::wl_listener
    events::ANONYMOUS1_events
    data::Ptr{Cvoid}
end

@cenum wlr_pointer_constraint_v1_type::UInt32 begin
    WLR_POINTER_CONSTRAINT_V1_LOCKED = 0
    WLR_POINTER_CONSTRAINT_V1_CONFINED = 1
end

@cenum wlr_pointer_constraint_v1_state_field::UInt32 begin
    WLR_POINTER_CONSTRAINT_V1_STATE_REGION = 1
    WLR_POINTER_CONSTRAINT_V1_STATE_CURSOR_HINT = 2
end


struct ANONYMOUS2_cursor_hint
    x::Cdouble
    y::Cdouble
end

struct wlr_pointer_constraint_v1_state
    committed::UInt32
    region::pixman_region32_t
    cursor_hint::ANONYMOUS2_cursor_hint
end

struct ANONYMOUS3_events
    new_constraint::wl_signal
end

struct wlr_pointer_constraints_v1
    _global::Ptr{wl_global}
    constraints::wl_list
    events::ANONYMOUS3_events
    display_destroy::wl_listener
    data::Ptr{Cvoid}
end
