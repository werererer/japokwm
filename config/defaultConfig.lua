require "tile"

-- TODO: config files must print error messages
-- you can find names in keysym.jl
mods = {"Shift_L", "Caps_Lock", "Control_L", "Alt_L", "", "", "Super_L", "ISO_Level3_Shift"}
mod1 = mods[4]
-- TODO what position in mods array?
mod2 = "Num_Lock"
mod3 = mods[2]
mod4 = mods[7]
-- also known as Alt Gr
mod5 = mods[8]
btn_left = "Pointer_Button1"
btn_right = "Pointer_Button2"
btn_middle = "Pointer_Button3"
shift = mods[1]
ctrl = mods[3]

cursor_mode = {
    CUR_NORMAL = 0,
    CUR_MOVE = 1,
    CUR_RESIZE = 2,
}

layouts = {
    { "[M]", function() monocle() end },
    { "[]=", function() two_pane() end },
    { "><>", function() floating() end },
    { "gf", function() load_layout("tmp") end },
}
layout = layouts[layout_id]

function set_layout()
    layout_id = layout_id + 1
    if layout_id > #layouts then
        layout_id = 1
    end
    mylib.update_layout()
end

function set_layout(i)
    print("set layout")
    layout_id = i
    layout = layouts[i]
    action.update_layout()
end

function toggle_overlay()
    overlay = action.get_overlay()
    action.set_overlay(not overlay)
    action.arrange_this(false);
end

-- default
mod = mod1
