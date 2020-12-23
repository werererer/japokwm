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
btnLeft = "Pointer_Button1"
btnRight = "Pointer_Button2"
btnMiddle = "Pointer_Button3"
shift = mods[1]
ctrl = mods[3]

cursorMode = {
    CurNormal = 0,
    CurMove = 1,
    CurResize = 2,
}

layouts = {
    { "[M]", function() monocle() end },
    { "[]=", function() twoPane() end },
    { "><>", function() floating() end },
    { "gf", function() loadLayout("tmp") end },
}
layout = layouts[layoutId]

function setLayout()
    layoutId = layoutId + 1
    if layoutId > #layouts then
        layoutId = 1
    end
    mylib.updateLayout()
end

function setLayout(i)
    print("set layout")
    layoutId = i
    layout = layouts[i]
    action.update_layout()
end

function toggleOverlay()
    overlay = action.get_overlay()
    action.set_overlay(not overlay)
    action.arrange_this(false);
end

-- default
mod = mod1
