#/usr/bin/juliaHool
#=
# This file wrapps c functions to julia
=#
const corePath = "./juliawm.so"

function run(cmd::String)
    ccall((:execute, corePath), Cvoid, (Cstring,), cmd)
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

function zoom()
    ccall((:zoom, corePath), Cvoid, ())
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

function togglefloating()
    ccall((:togglefloating, corePath), Cvoid, ())
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

function moveResize(ui :: Int)
    ccall((:moveResize, corePath), Cvoid, (Cuint,), convert(Cuint, ui))
end

function quit()
    ccall((:quit, corePath), Cvoid, ())
end
