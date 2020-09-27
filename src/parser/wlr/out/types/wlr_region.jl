# Julia wrapper for header: wlr_region.h
# Automatically generated using Clang.jl


function wlr_region_create(client, version, id, resource_list)
    ccall((:wlr_region_create, wlr_region), Ptr{wl_resource}, (Ptr{wl_client}, UInt32, UInt32, Ptr{wl_list}), client, version, id, resource_list)
end

function wlr_region_from_resource(resource)
    ccall((:wlr_region_from_resource, wlr_region), Ptr{pixman_region32_t}, (Ptr{wl_resource},), resource)
end
