include("hotkeyUtils.jl")

function keybinding(mod, sym)
    mods = modToString(mod)
    key = XKeysymToString(sym)
    bind = "$(mods)$(key)"
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
