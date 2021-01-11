require "tile"

-- TODO: config files must print error messages
-- you can find names in keysym.jl
Mods = {"Shift_L", "Caps_Lock", "Control_L", "Alt_L", "", "", "Super_L", "ISO_Level3_Shift"}
Mod1 = Mods[4]
-- TODO what position in mods array?
Mod2 = "Num_Lock"
Mod3 = Mods[2]
Mod4 = Mods[7]
-- also known as Alt Gr
Mod5 = Mods[8]
Btn_left = "Pointer_Button1"
Btn_right = "Pointer_Button2"
Btn_middle = "Pointer_Button3"
Shift = Mods[1]
Ctrl = Mods[3]

Cursor_mode = {
    CUR_NORMAL = 0,
    CUR_MOVE = 1,
    CUR_RESIZE = 2,
}

Layouts = {
    { "[M]", function() monocle() end },
    { "[]=", function() two_pane() end },
    { "><>", function() floating() end },
    { "gf", function() Load_layout("tmp") end },
}
-- TODO why can't I rename this variable?
layout = Layouts[layout_id]

function set_layout()
    layout_id = layout_id + 1
    if layout_id > #Layouts then
        layout_id = 1
    end
    action.update_layout()
end

function set_layout(i)
    layout_id = i
    layout = Layouts[i]
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
mod = Mod1
