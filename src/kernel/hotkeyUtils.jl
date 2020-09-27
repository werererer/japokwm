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

