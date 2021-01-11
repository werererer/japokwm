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
    {"[M]", function() Load_layout("master") end},
    {"[]=", function() Load_layout("two_pane") end},
    {"||",  function() Load_layout("monocle") end},
    {"--",  function() Load_layout("tmp") end },
}

Layout_id = 1
-- TODO why can't I rename this variable?
layout = Layouts[Layout_id]

function Set_layout()
    Layout_id = Layout_id + 1
    if Layout_id > #Layouts then
        Layout_id = 1
    end
    action.update_layout()
end

function Set_layout(i)
    Layout_id = i
    layout = Layouts[i]
    layout[2]()
    action.update_layout()
    action.arrange_this(false)
end

-- default
Mod = Mod1
