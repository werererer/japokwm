# TODO: CLEANMASK
module keys
include("config.jl")

function stringToKeysym(str)
    return ccall((:StringToKeysym, "./keys.so"), Cint, (Cstring,), str)
end


function cleanmask(mask)
    return ccall((:cleanmask, "./dwl.so"), Cint, (Cint), mask)
end


function keybinding(mods, sym)
    handled = false
    for key in keys
        for C-Shifht
        if (cleanmask(mods) == parseKernel.cleanmask(key.mod)) &&
            (sym == key.keysym && k.func)
            key.func(keys.key)
            handled = true
        end
    end
    return handled
end
