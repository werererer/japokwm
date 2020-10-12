module Layouts
import Statistics

# from 0-1
struct wlr_fbox
    x :: Cdouble
    y :: Cdouble
    width :: Cdouble
    height :: Cdouble
end

mutable struct Monitor
    m :: wlr_fbox # monitor area
    w :: wlr_fbox # window area
    tagset :: Int
    mfact :: Float64
    nmaster :: Int
end

struct cLayoutArr
    layout :: Ptr{wlr_fbox}
    size :: Cint
end

function add(box :: wlr_fbox)
    ccall((:addBox, "juliawm.so"), Cvoid, (Cint, Cint, Cint, Cint), 3, 3, 4, 5)
end

function del()
    ccall((:del, "juliawm.so"), Cvoid, (Cint,), 3)
end

function recurse(arr :: Array{Array{Array{Int}}})
    a = arr[size(arr)] 
end

# set: which window conf set
# client: current window
function split(arr, set :: Int, client :: Int, ratio :: Float64)
    arrange = arr[set]
    space = arr[set][client]
    w = space[4]

    space[4] *= ratio
    newSpace = copy(space)
    newSpace[2] = space[2] + space[4]
    newSpace[4] = w * (1-ratio)
    push!(arrange, newSpace)
end

# set: which window conf set
# client: current window
function vsplit(arr, set :: Int, client :: Int, ratio :: Float64)
    arrange = arr[set]
    space = arr[set][client]
    h = space[3]

    space[3] *= ratio
    newSpace = copy(space)
    newSpace[1] = space[1] + space[3]
    newSpace[3] = h * (1-ratio)
    push!(arrange, newSpace)
end

function tile(n) :: Ptr{cLayoutArr}
    println("tile")
    layout = [
           [
            wlr_fbox(0, 0, 1, 1),
            wlr_fbox(2, 2, 1, 1),
           ],
           [
            wlr_fbox(0, 0, 1/2, 1),
            wlr_fbox(1/2, 0, 1/2, 1),
           ],
           [
            wlr_fbox(0, 0, 1/2, 1),
            wlr_fbox(0, 0, 1/2, 1),
            wlr_fbox(1/2, 0, 1/2, 1),
           ],
           [
            wlr_fbox(0, 0, 1/2, 1),
            wlr_fbox(0, 0, 1/2, 1),
            wlr_fbox(1/2, 0, 1/2, 1),
           ],
          ]

    res = cLayoutArr(pointer(layout[1]), length(layout[1]))
    res = pointer([res])
    return res
end

function monocle(n) :: Ptr{cLayoutArr}
    println("monocle")
    layout = [[
            wlr_fbox(1/3, 1/2, 1/3, 1/2)
           ]]

    res = cLayoutArr(pointer(layout[1]), length(layout[1]))
    res = pointer([res])
    return res
end

end
