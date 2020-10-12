include("kernel/parseKernel.jl")
include("keysym.jl")
include("kernel/tile.jl")

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
const shift = mods[1]
const ctrl = mods[3]
#
@enum Screentransform begin
    NORMAL
end

function setLayout(l)
    println("NEWLAYOUT")
    println(l)
    global layout = l
    updateLayout()
end

#default
mod = mod1
