require "tile"

-- TODO: config files must print error messages
-- you can find names in keysym.jl
Mods = {"Shift_L", "Caps_Lock", "Control_L", "Alt_L", "", "", "Super_L", "ISO_Level3_Shift"}
mod1 = Mods[4]
-- TODO what position in mods array?
mod2 = "Num_Lock"
mod3 = Mods[2]
mod4 = Mods[7]
-- also known as Alt Gr
mod5 = Mods[8]
btn_left = "Pointer_Button1"
btn_right = "Pointer_Button2"
btn_middle = "Pointer_Button3"
shift = Mods[1]
ctrl = Mods[3]

cursor_mode = {
    CUR_NORMAL = 0,
    CUR_MOVE = 1,
    CUR_RESIZE = 2,
}

layouts = {
    { "[M]", function() monocle() end },
    { "[]=", function() two_pane() end },
    { "><>", function() floating() end },
    { "gf", function() Load_layout("tmp") end },
}
layout = layouts[layout_id]

function set_layout()
    layout_id = layout_id + 1
    if layout_id > #layouts then
        layout_id = 1
    end
    action.update_layout()
end

function set_layout(i)
    layout_id = i
    layout = layouts[i]
    layout[2]()
    action.update_layout()
    action.arrange_this(false)
end

function toggle_overlay()
    overlay = action.get_overlay()
    action.set_overlay(not overlay)
    action.arrange_this(false);
end

-- default
mod = mod1
