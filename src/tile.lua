-- include("translationLayer.jl")
-- include("parseLayout.jl")

-- function add(box)
--     ccall((:addBox, corePath), Cvoid, (Cint, Cint, Cint, Cint), 3, 3, 4, 5)
-- end

-- function del()
--     ccall((:del, corePath), Cvoid, (Cint,), 3)
-- end

-- function recurse(arr)
--     a = arr[size(arr)] 
-- end

layoutData = {
    {
        {0, 0, 1, 1},
    },
    {
        {0, 0, 1, 1},
        {0, 0, 1, 1},
    },
}

-- set: which window conf set
-- client: current window
function splitContainer(i, j, ratio)
    local i = min(i, length(layoutData))
    local j = min(j, length(layoutData[i]))
    container = layoutData[i][j]

    prevHeight = container.height
    container = {
        container.x, container.y, container.width,
        container.height * ratio
    }
    newContainer = {
        container.x, container.y + container.height,
        container.width, prevHeight * (1-ratio)
    }
    layoutData[i][j] = container

    table.insert(layoutData, i+1, layoutData[i])
    table.insert(layoutData, newContainer)
    arrangeThis(false)
end

function splitThisContainer(ratio)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    splitContainer(i, j, ratio)
end

-- set: which window conf set
-- client: current window
function vsplitContainer(i, j, ratio)
    local i = min(i, length(layoutData))
    local j = min(j, length(layoutData[i]))
    container = layoutData[i][j]

    prevWidth = container.width
    container = Container(container.x, container.y, container.width * ratio,
                          container.height)
    newContainer = Container(container.x + container.width, container.y,
                             prevWidth * (1-ratio), container.height)
    layoutData[i][j] = container

    table.insert(layoutData, i+1, layoutData[i])
    table.insert(layoutData, newContainer)
    arrangeThis(false)
end

function vsplitThisContainer(ratio)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    vsplitContainer(i, j, ratio)
end

function mergeContainer(i, j1, j2)
    if i > length(layoutData) then
        return
    end
    if max(j1, j2) > length(layoutData[i]) then
        return
    end

    local i = min(i, length(layoutData))
    local j1 = min(j1, length(layoutData[i]))
    local j2 = min(j2, length(layoutData[i]))
    local container1 = layoutData[i][j1]
    local container2 = layoutData[i][j2]

    local x = min(container1.x, container2.x)
    local y = min(container1.y, container2.y)
    local width = max(container1.x + container1.width,
                container2.x + container2.width) - x
    local height = max(container1.y + container1.height,
                container2.y + container2.height) - y
    local newContainer = Container(x, y, width, height)

    layoutData[i][min(j1, j2)] = newContainer
    layoutData[i] = max(j1, j2)
    arrangeThis(false)
end

Direction = {
    TOP = 1,
    BOTTOM = 2,
    LEFT = 3,
    RIGHT = 4,
}

function moveContainer(i, j, n, d)
    container = layoutData[i][j]
    if d == Direction.TOP then
        layoutData[i][j] = Container(container.x, container.y - n, container.width, container.height)
    elseif d == Direction.BOTTOM then
        layoutData[i][j] = Container(container.x, container.y + n, container.width, container.height)
    elseif d == Direction.LEFT then
        layoutData[i][j] = Container(container.x - n, container.y, container.width, container.height)
    elseif d == Direction.RIGHT then
        layoutData[i][j] = Container(container.x + n, container.y, container.width, container.height)
    end
end

function moveThisContainer(n, d)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    moveContainer(i, j, n, d)
    arrangeThis(false)
end

function resizeContainer(i, j, n, d)
    container = layoutData[i][j]
    if d == Direction.TOP then
        layoutData[i][j] = Container(container.x, container.y - n,
                                     container.width, container.height + n)
    elseif d == Direction.BOTTOM then
        layoutData[i][j] = Container(container.x, container.y,
                                     container.width, container.height + n)
    elseif d == Direction.LEFT then
        layoutData[i][j] = Container(container.x - n, container.y,
                                     container.width + n, container.height)
    elseif d == Direction.RIGHT then
        layoutData[i][j] = Container(container.x, container.y,
                                     container.width + n, container.height)
    end
end

function resizeThisContainer(n, d)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    resizeContainer(i, j, n, d)
    arrangeThis(false)
end

function resizeAll(i, j, n, d)
    local container = layoutData[i][j]
    for j2 in range(1,length(layoutData[i])) do
        if j == j2 then
            resizeContainer(i, j, n, d)
        else
            -- resize container[i][j]?
            if d == Direction.TOP then
                resize =
                layoutData[i][j2].y + layoutData[i][j2].height <= container.y
            elseif d == Direction.BOTTOM then
                resize = layoutData[i][j2].y >= container.y + container.width
            elseif d == Direction.LEFT then
                resize =
                layoutData[i][j2].x + layoutData[i][j2].width <= container.x
            elseif d == Direction.RIGHT then
                resize = layoutData[i][j2].x >= container.x + container.width
            end

            if resize then
                resizeContainer(i, j2, -n, d)
                moveContainer(i, j2, n, d)
            end
        end
    end
end

function resizeThisAll(n, d)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    resizeAll(i, j, n, d)
    arrangeThis(false)
end

function tile(n)
    layoutData = action.readOverlay("tile")
end

function monocle(n)
    layoutData = action.readOverlay("monocle")
end

function twoPane(n)
    layoutData = action.readOverlay("twoPane")
end

function update(n)
    print("update", #layoutData)
    local i = math.max(math.min(#layoutData, n), 1)
    return layoutData[i]
end
