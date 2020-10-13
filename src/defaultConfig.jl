include("keysym.jl")
include("core/tile.jl")
include("core/translationLayer.jl")

#TODO: config files must print error messages
#you can find names in keysym.jl
const mods = ["Shift_L", "Caps_Lock", "Control_L", "Alt_L", "", "", "Super_L", "ISO_Level3_Shift"]
const mod1 = mods[4]
#TODO what position in mods array?
const mod2 = "Num_Lock"
const mod3 = mods[2]
const mod4 = mods[7]
#also known as Alt Gr
const mod5 = mods[8]
const btnLeft = "Pointer_Button1"
const btnRight = "Pointer_Button2"
const btnMiddle = "Pointer_Button3"
const shift = mods[1]
const ctrl = mods[3]
#
@enum Screentransform begin
    NORMAL
end

function setLayout(l)
    global layout = l
    updateLayout()
end

#default
mod = mod1
