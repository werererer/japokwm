module Layouts
import Statistics
include("translationLayer.jl")

# from 0-1
struct Container
    x :: Cdouble
    y :: Cdouble
    width :: Cdouble
    height :: Cdouble
end

mutable struct Monitor
    m :: Container # monitor area
    w :: Container # window area
    tagset :: Int
    mfact :: Float64
    nmaster :: Int
end

struct cContainerArr
    container :: Ptr{Container}
    size :: Cint
end

function add(box :: Container)
    ccall((:addBox, corePath), Cvoid, (Cint, Cint, Cint, Cint), 3, 3, 4, 5)
end

function del()
    ccall((:del, corePath), Cvoid, (Cint,), 3)
end

function recurse(arr :: Array{Array{Array{Int}}})
    a = arr[size(arr)] 
end

layoutData = [[Container(0, 0, 1, 1)]]

# set: which window conf set
# client: current window
function splitContainer(i :: Int, j :: Int, ratio :: Float64)
    global layoutData
    i = min(i, length(layoutData))
    j = min(j, length(layoutData[i]))
    container = layoutData[i][j]

    prevHeight = container.height
    container = Container(container.x, container.y, container.width,
                          container.height * ratio)
    newContainer = Container(container.x, container.y + container.height,
                             container.width, prevHeight * (1-ratio))
    layoutData[i][j] = container

    insert!(layoutData, i+1, layoutData[i])
    push!(layoutData[i+1], newContainer)
    arrangeThis(false)
end

function splitThisContainer(ratio :: Float64)
    i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    j = min(clientPos(), length(layoutData[i]))
    splitContainer(i, j, ratio)
end

# set: which window conf set
# client: current window
function vsplitContainer(i :: Int, j :: Int, ratio :: Float64)
    global layoutData
    i = min(i, length(layoutData))
    j = min(j, length(layoutData[i]))
    container = layoutData[i][j]

    prevWidth = container.width
    container = Container(container.x, container.y, container.width * ratio,
                          container.height)
    newContainer = Container(container.x + container.width, container.y,
                             prevWidth * (1-ratio), container.height)
    layoutData[i][j] = container

    insert!(layoutData, i+1, layoutData[i])
    push!(layoutData[i+1], newContainer)
    arrangeThis(false)
end

function vsplitThisContainer(ratio :: Float64)
    i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    j = min(clientPos(), length(layoutData[i]))
    vsplitContainer(i, j, ratio)
end

function mergeContainer(i :: Int, j1 :: Int, j2 :: Int)
    global layoutData

    if i > length(layoutData)
        return
    end
    if max(j1, j2) > length(layoutData[i])
        return
    end

    i = min(i, length(layoutData))
    j1 = min(j1, length(layoutData[i]))
    j2 = min(j2, length(layoutData[i]))
    container1 = layoutData[i][j1]
    container2 = layoutData[i][j2]

    x = min(container1.x, container2.x)
    y = min(container1.y, container2.y)
    width = max(container1.x + container1.width,
                container2.x + container2.width) - x
    height = max(container1.y + container1.height,
                container2.y + container2.height) - y
    newContainer = Container(x, y, width, height)

    layoutData[i][min(j1, j2)] = newContainer
    deleteat!(layoutData[i], max(j1, j2))
    arrangeThis(false)
end

@enum Direction begin
    TOP
    BOTTOM
    LEFT
    RIGHT
end

function moveContainer(i :: Int, j :: Int, n :: Float64, d :: Direction)
    global layoutData
    container = layoutData[i][j]
    if d == TOP
        layoutData[i][j] = Container(container.x, container.y - n, container.width, container.height)
    elseif d == BOTTOM
        layoutData[i][j] = Container(container.x, container.y + n, container.width, container.height)
    elseif d == LEFT
        layoutData[i][j] = Container(container.x - n, container.y, container.width, container.height)
    elseif d == RIGHT
        layoutData[i][j] = Container(container.x + n, container.y, container.width, container.height)
    end
end

function moveThisContainer(n :: Float64, d :: Direction)
    i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    j = min(clientPos(), length(layoutData[i]))
    moveContainer(i, j, n, d)
    arrangeThis(false)
end

function resizeContainer(i :: Int, j :: Int, n :: Float64, d :: Direction)
    global layoutData
    container = layoutData[i][j]
    if d == TOP
        layoutData[i][j] = Container(container.x, container.y - n,
                                     container.width, container.height + n)
    elseif d == BOTTOM
        layoutData[i][j] = Container(container.x, container.y,
                                     container.width, container.height + n)
    elseif d == LEFT
        layoutData[i][j] = Container(container.x - n, container.y,
                                     container.width + n, container.height)
    elseif d == RIGHT
        layoutData[i][j] = Container(container.x, container.y,
                                     container.width + n, container.height)
    end
end

function resizeThisContainer(n :: Float64, d :: Direction)
    global layoutData
    i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    j = min(clientPos(), length(layoutData[i]))
    resizeContainer(i, j, n, d)
    arrangeThis(false)
end

function resizeAll(i :: Int, j :: Int, n :: Float64, d :: Direction)
    global layoutData
    container = layoutData[i][j]
    for j2 in 1:length(layoutData[i])
        if j == j2
            resizeContainer(i, j, n, d)
            continue
        end

        #resize container[i][j]?
        if d == TOP
            resize =
            layoutData[i][j2].y + layoutData[i][j2].height <= container.y
        elseif d == BOTTOM
            resize = layoutData[i][j2].y >= container.y + container.width
        elseif d == LEFT
            resize =
            layoutData[i][j2].x + layoutData[i][j2].width <= container.x
        elseif d == RIGHT
            resize = layoutData[i][j2].x >= container.x + container.width
        end

        if resize
            resizeContainer(i, j2, -n, d)
            moveContainer(i, j2, n, d)
        end
    end
end

function resizeThisAll(n :: Float64, d :: Direction)
    global layoutData
    i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    j = min(clientPos(), length(layoutData[i]))
    resizeAll(i, j, n, d)
    arrangeThis(false)
end

function tile(n :: Int) :: Ptr{cContainerArr}
    global layoutData

    layoutData = [
                  [
                   Container(0, 0, 1, 1);
                  ],
                  [
                   Container(0, 0, 1/2, 1),
                   Container(1/2, 0, 1/2, 1),
                  ],
                  [
                   Container(0, 0, 1/2, 1),
                   Container(1/2, 0, 1/2, 1/2),
                   Container(1/2, 1/2, 1/2, 1/2),
                  ],
                  [
                   Container(0, 0, 1/2, 1),
                   Container(1/2, 0, 1/2, 1/3),
                   Container(1/2, 1/3, 1/2, 1/3),
                   Container(1/2, 2/3, 1/2, 1/3),
                  ],
                  [
                   Container(0  , 0  , 1/2, 1  ),
                   Container(1/2, 0  , 1/2, 1/4),
                   Container(1/2, 1/4, 1/2, 1/4),
                   Container(1/2, 2/4, 1/2, 1/4),
                   Container(1/2, 3/4, 1/2, 1/4),
                  ],
                 ]
    return update(n)
end

function monocle(n :: Int) :: Ptr{cContainerArr}
    global layoutData = [[
            Container(1/3, 1/2, 2/3, 1/2)
           ]]

    return update(n)
end

# return array of arrangement
function update(n :: Int) :: Ptr{cContainerArr}
    global layoutData
    n = max(1, min(n, length(layoutData)))
    res = cContainerArr(pointer(layoutData[n]), length(layoutData[n]))
    res = pointer([res])
    return res
end
end
