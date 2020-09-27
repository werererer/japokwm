# Julia wrapper for header: drm_format_set.h
# Automatically generated using Clang.jl


function wlr_drm_format_set_finish(set)
    ccall((:wlr_drm_format_set_finish, drm_format_set), Cvoid, (Ptr{wlr_drm_format_set},), set)
end

function wlr_drm_format_set_get(set, format)
    ccall((:wlr_drm_format_set_get, drm_format_set), Ptr{wlr_drm_format}, (Ptr{wlr_drm_format_set}, UInt32), set, format)
end

function wlr_drm_format_set_has(set, format, modifier)
    ccall((:wlr_drm_format_set_has, drm_format_set), Bool, (Ptr{wlr_drm_format_set}, UInt32, UInt64), set, format, modifier)
end

function wlr_drm_format_set_add(set, format, modifier)
    ccall((:wlr_drm_format_set_add, drm_format_set), Bool, (Ptr{wlr_drm_format_set}, UInt32, UInt64), set, format, modifier)
end
