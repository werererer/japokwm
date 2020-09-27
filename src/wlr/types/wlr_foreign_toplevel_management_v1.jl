# Julia wrapper for header: wlr_foreign_toplevel_management_v1.h
# Automatically generated using Clang.jl


function wlr_foreign_toplevel_manager_v1_create(display)
    ccall((:wlr_foreign_toplevel_manager_v1_create, wlr_foreign_toplevel_management_v1), Ptr{wlr_foreign_toplevel_manager_v1}, (Ptr{wl_display},), display)
end

function wlr_foreign_toplevel_handle_v1_create(manager)
    ccall((:wlr_foreign_toplevel_handle_v1_create, wlr_foreign_toplevel_management_v1), Ptr{wlr_foreign_toplevel_handle_v1}, (Ptr{wlr_foreign_toplevel_manager_v1},), manager)
end

function wlr_foreign_toplevel_handle_v1_destroy(toplevel)
    ccall((:wlr_foreign_toplevel_handle_v1_destroy, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1},), toplevel)
end

function wlr_foreign_toplevel_handle_v1_set_title(toplevel, title)
    ccall((:wlr_foreign_toplevel_handle_v1_set_title, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Cstring), toplevel, title)
end

function wlr_foreign_toplevel_handle_v1_set_app_id(toplevel, app_id)
    ccall((:wlr_foreign_toplevel_handle_v1_set_app_id, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Cstring), toplevel, app_id)
end

function wlr_foreign_toplevel_handle_v1_output_enter(toplevel, output)
    ccall((:wlr_foreign_toplevel_handle_v1_output_enter, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Ptr{wlr_output}), toplevel, output)
end

function wlr_foreign_toplevel_handle_v1_output_leave(toplevel, output)
    ccall((:wlr_foreign_toplevel_handle_v1_output_leave, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Ptr{wlr_output}), toplevel, output)
end

function wlr_foreign_toplevel_handle_v1_set_maximized(toplevel, maximized)
    ccall((:wlr_foreign_toplevel_handle_v1_set_maximized, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Bool), toplevel, maximized)
end

function wlr_foreign_toplevel_handle_v1_set_minimized(toplevel, minimized)
    ccall((:wlr_foreign_toplevel_handle_v1_set_minimized, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Bool), toplevel, minimized)
end

function wlr_foreign_toplevel_handle_v1_set_activated(toplevel, activated)
    ccall((:wlr_foreign_toplevel_handle_v1_set_activated, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Bool), toplevel, activated)
end

function wlr_foreign_toplevel_handle_v1_set_fullscreen(toplevel, fullscreen)
    ccall((:wlr_foreign_toplevel_handle_v1_set_fullscreen, wlr_foreign_toplevel_management_v1), Cvoid, (Ptr{wlr_foreign_toplevel_handle_v1}, Bool), toplevel, fullscreen)
end
