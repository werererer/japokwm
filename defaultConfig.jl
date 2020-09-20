#TODO: print error message
include("kernel/parseKernel.jl")
include("keysym.jl")


#you can find names in xkbcommon-keysyms.h
const mod1 = "Alt_L"
const mod2 = "Num_Lock"
const mod3 = "Caps_Lock"
const mod4 = "Super_L"
#also known as Alt Gr
const mod5 = "ISO_Level3_Shift"

@enum Screentransform begin
    NORMAL
end

#default
mod = mod1
shift = "Shift_L"
