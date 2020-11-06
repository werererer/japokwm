module juliawm
include("keybinding.jl");
include("keysym.jl")

function setup()
    ccall((:setup, config.corePath), Cvoid, ())
end

function run(cmd::String)
    return ccall((:run, config.corePath), Cvoid, (Cstring,), cmd)
end

function cleanup()
    ccall((:cleanup, config.corePath), Cvoid, ())
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

    config.updateConfig()
    setup()
    run(startup_cmd)
    cleanup()
    return 0
end
julia_main()
end
