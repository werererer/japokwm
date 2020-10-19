#/usr/bin/juliaHool
#=
# This file wrapps c functions to julia
=#
include("global.jl")
const corePath = "./libjuliawm.so"

function arrangeThis(reset :: Bool)
    ccall((:arrangeThis, corePath), Cvoid, (Cint,), reset)
end

function spawn(cmd :: String)
    ccall((:spawn, corePath), Cvoid, (Cstring,), cmd)
end

function focusstack(i)
    ccall((:focusstack, corePath), Cvoid, (Cint,), i)
end

function incnmaster(i)
    ccall((:incnmaster, corePath), Cvoid, (Cint,), i)
end

function setmfact(factor)
    ccall((:setmfact, corePath), Cvoid, (Cfloat,), factor)
end

function updateConfig()
    ccall((:updateConfig, corePath), Cvoid, ())
end

function zoom()
    ccall((:zoom, corePath), Cvoid, ())
end

function thisTiledClientCount() :: Int
    return ccall((:thisTiledClientCount, corePath), Cint, ())
    return i
end

function clientPos() :: Int
    # julia is 1 based, c 0
    return ccall((:clientPos, corePath), Cint, ()) + 1
end

function view(ui)
    ccall((:view, corePath), Cvoid, (Cint,), ui)
end

function killclient()
    ccall((:killclient, corePath), Cvoid, ())
end

function updateLayout()
    ccall((:updateLayout, corePath), Cvoid, ())
end

function toggleFloating()
    ccall((:toggleFloating, corePath), Cvoid, ())
end

function tag(ui)
    ccall((:tag, corePath), Cvoid, (Cint,), ui)
end

function focusmon(i)
    ccall((:focusmon, corePath), Cvoid, (Cint,), i)
end

function tagmon(i)
    ccall((:tagmon, corePath), Cvoid, (Cint,), i)
end

function moveResize(ui :: Cursor)
    ccall((:moveResize, corePath), Cvoid, (Cuint,), convert(Cuint, Int(ui)))
end

function quit()
    ccall((:quit, corePath), Cvoid, ())
end

function createOverlay()
    ccall((:createOverlay, corePath), Cvoid, ())
end

function createNewOverlay()
    ccall((:createNewOverlay, corePath), Cvoid, ())
end

function setOverlay(overlay :: Bool)
    ccall((:setOverlay, corePath), Cvoid, (Cint,), overlay)
end

function getOverlay() :: Bool
    ccall((:getOverlay, corePath), Bool, ())
end
