struct Monitor
    x::Int
    y::Int
    width::Int
    height::Int
end


function wlr_output_layout_get_box(m::Monitor)::wlr_box
    #TODO: type?
    ccall((:wlr_output_layout_get_box,"./wayland-util.so"), CVoid, (Cint, Cint))
end
