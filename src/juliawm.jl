module juliawm
include("translationLayer.jl")
include("keybinding.jl");
include("keysym.jl")

function setup()
    ccall((:setup, corePath), Cvoid, ())
end

function run(cmd::String)
    return ccall((:run, corePath), Cvoid, (Cstring,), cmd)
end

function cleanup()
    ccall((:cleanup, corePath), Cvoid, ())
end

function julia_main() :: Cint
    startup_cmd = ""
    c = 0

    if length(ARGS) > 0
        startup_cmd = ARGS[1]
    end
    println(startup_cmd)

    if isempty(ENV["XDG_RUNTIME_DIR"])
        println("XDG_RUNTIME_DIR must be set")
    end

    updateConfig()
    setup()
    run(startup_cmd)
    cleanup()
    return 0
end
end
