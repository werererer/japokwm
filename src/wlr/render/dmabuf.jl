# Julia wrapper for header: dmabuf.h
# Automatically generated using Clang.jl


function wlr_dmabuf_attributes_finish(attribs)
    ccall((:wlr_dmabuf_attributes_finish, dmabuf), Cvoid, (Ptr{wlr_dmabuf_attributes},), attribs)
end

function wlr_dmabuf_attributes_copy(dst, src)
    ccall((:wlr_dmabuf_attributes_copy, dmabuf), Bool, (Ptr{wlr_dmabuf_attributes}, Ptr{wlr_dmabuf_attributes}), dst, src)
end
nes::Cint
    offset::NTuple{4, UInt32}
    stride::NTuple{4, UInt32}
    fd::NTuple{4, Cint}
end
