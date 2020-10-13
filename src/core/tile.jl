module Layouts
import Statistics

# from 0-1
struct LayoutElement
    x :: Cdouble
    y :: Cdouble
    width :: Cdouble
    height :: Cdouble
end

mutable struct Monitor
    m :: LayoutElement # monitor area
    w :: LayoutElement # window area
    tagset :: Int
    mfact :: Float64
    nmaster :: Int
end

struct cLayoutArr
    layout :: Ptr{LayoutElement}
    size :: Cint
end

function add(box :: LayoutElement)
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
               LayoutElement(0, 0, 1, 1);
              ],
              [
               LayoutElement(0, 0, 1/2, 1),
               LayoutElement(1/2, 0, 1/2, 1),
              ],
              [
               LayoutElement(0, 0, 1/2, 1),
               LayoutElement(1/2, 0, 1/2, 1/2),
               LayoutElement(1/2, 1/2, 1/2, 1/2),
              ],
              [
               LayoutElement(0, 0, 1/2, 1),
               LayoutElement(1/2, 0, 1/2, 1/3),
               LayoutElement(1/2, 1/3, 1/2, 1/3),
               LayoutElement(1/2, 2/3, 1/2, 1/3),
              ],
              [
               LayoutElement(0  , 0  , 1/2, 1  ),
               LayoutElement(1/2, 0  , 1/2, 1/4),
               LayoutElement(1/2, 1/4, 1/2, 1/4),
               LayoutElement(1/2, 2/4, 1/2, 1/4),
               LayoutElement(1/2, 3/4, 1/2, 1/4),
              ],
             ]

    n = max(1, min(n, length(layout)))
    res = cLayoutArr(pointer(layout[n]), length(layout[n]))
    res = pointer([res])
    return res
end

function monocle(n :: Int) :: Ptr{cLayoutArr}
    layout = [[
            LayoutElement(1/3, 1/2, 1/3, 1/2)
           ]]

    println("monocle")
    n = max(1, min(n, length(layout)))
    res = cLayoutArr(pointer(layout[n]), length(layout[n]))
    res = pointer([res])
    return res
end

end
