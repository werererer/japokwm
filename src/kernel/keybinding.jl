include("../config.jl")
const ULIB = "./wayland-util.so"

function XStringToKeysym(str::String)
    return ccall((:XStringToKeysym, ULIB), Cint, (Cstring,), str)
end

function XKeysymToString(sym)
    res = ccall((:XKeysymToString, ULIB), Cstring, (Cint,), sym)
    return unsafe_string(res)
end

function modToString(mod)
    res = ""
    mask = x->2^(x-1)
    for i in 1:8
        if mod & mask(i) != 0
            res *= "$(mods[i]) "
        end
    end
    return res
end

function keybinding(mod, sym)
    mods = modToString(mod)
    key = XKeysymToString(sym)
    bind = "$(mods)$(key)"
    println("BIND: $bind")
    return keybinding(bind)
end

function keybinding(bind::String)
    handled = false
    for key in keys
        if (bind == key[1])
            #call function saved in key[2]
            key[2]()
            handled = true
        end
    end
    return handled
end

function buttonPress(mods, btn)
    for hotkey in hotkeys
        if modToString(mod) == hotkey->mod
        end
    end
end
