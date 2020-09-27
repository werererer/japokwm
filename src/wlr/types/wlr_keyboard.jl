# Julia wrapper for header: wlr_keyboard.h
# Automatically generated using Clang.jl


function wlr_keyboard_set_keymap(kb, keymap)
    ccall((:wlr_keyboard_set_keymap, wlr_keyboard), Bool, (Ptr{wlr_keyboard}, Ptr{xkb_keymap}), kb, keymap)
end

function wlr_keyboard_keymaps_match(km1, km2)
    ccall((:wlr_keyboard_keymaps_match, wlr_keyboard), Bool, (Ptr{xkb_keymap}, Ptr{xkb_keymap}), km1, km2)
end

function wlr_keyboard_set_repeat_info(kb, rate, delay)
    ccall((:wlr_keyboard_set_repeat_info, wlr_keyboard), Cvoid, (Ptr{wlr_keyboard}, Int32, Int32), kb, rate, delay)
end

function wlr_keyboard_led_update(keyboard, leds)
    ccall((:wlr_keyboard_led_update, wlr_keyboard), Cvoid, (Ptr{wlr_keyboard}, UInt32), keyboard, leds)
end

function wlr_keyboard_get_modifiers(keyboard)
    ccall((:wlr_keyboard_get_modifiers, wlr_keyboard), UInt32, (Ptr{wlr_keyboard},), keyboard)
end
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
    repeat_info::ANONYMOUS1_repeat_info
    events::ANONYMOUS2_events
    data::Ptr{Cvoid}
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
