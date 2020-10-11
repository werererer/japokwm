module Layouts
import Statistics

# from 0-1
mutable struct wlr_box
    x :: Int
    y :: Int
    width :: Int
    height :: Int
end
g = wlr_box(0, 0, 0, 0)

mutable struct Monitor
    m :: wlr_box # monitor area
    w :: wlr_box # window area
    tagset :: Int
    mfact :: Float64
    nmaster :: Int
end

function arrange()
    ccall((:arrange, "juliawm.so"), Cvoid, ())
end

function add(box :: wlr_box)
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
    println("works")
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

function tile(m :: Monitor)
    box = [
           [
            [0, 0, 1, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
           [
            [0, 0, 1/2, 1],
            [0, 0, 1/2, 1],
            [1/2, 0, 1/2, 1]
           ],
          ]
end

function monocle(m :: Monitor)
    println("works")
    box = [[[0, 0, 1, 1]]]
end

end
