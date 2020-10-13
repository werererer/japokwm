include("../config.jl")
const ULIB = "./wayland-util.so"

function XStringToKeysym(str :: String)
    return ccall((:XStringToKeysym, ULIB), Cint, (Cstring,), str)
end

function XKeysymToString(sym) :: String
    res = ccall((:XKeysymToString, ULIB), Cstring, (Cint,), sym)
    return unsafe_string(res)
end

function modToString(mod) :: String
    res = ""
    mask = x->2^(x-1)
    for i in 1:8
        if mod & mask(i) != 0
            res *= "$(mods[i]) "
        end
    end
    return res
end

function symToBinding(mod, sym) :: String
    mods = modToString(mod)
    key = XKeysymToString(sym)
    return "$(mods)$(key)"
end

function keyPressed(mod, sym)
    bind = symToBinding(mod, sym)
    println("bind: $bind")
    return processBinding(bind, keys)
end

function buttonPressed(mod, sym)
    bind = symToBinding(mod, sym)
    println("bind: $bind")
    return processBinding(bind, buttons)
end

function processBinding(bind :: String, arr)
    handled = false
    for elem in arr
        if (bind == elem[1])
            #call function saved in key[2]
            elem[2]()
            handled = true
        end
    end
    return handled
end
