# Julia wrapper for header: wlr_fullscreen_shell_v1.h
# Automatically generated using Clang.jl


function wlr_fullscreen_shell_v1_create(display)
    ccall((:wlr_fullscreen_shell_v1_create, wlr_fullscreen_shell_v1), Ptr{wlr_fullscreen_shell_v1}, (Ptr{wl_display},), display)
end
