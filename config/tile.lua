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
}

-- set: which window conf set
-- client: current window
function splitContainer(i, j, ratio)
    local i = math.min(i, #layoutData)
    local j = math.min(j, #layoutData[i])
    container = layoutData[i][j]

    print(i, j)
    x = container[1]
    y = container[2]
    width = container[3]
    height = container[4]

    container[4] = height * ratio
    newContainer = {x, y + container[4], width, height * (1-ratio)}

    table.insert(layoutData[i], newContainer)
    action.arrangeThis(false)
end

function splitThisContainer(ratio)
    local i = math.max(math.min(info.thisTiledClientCount(), #layoutData), 1)
    local j = math.min(info.thisContainerPosition(), #layoutData[i])
    splitContainer(i, j, ratio)
end

-- set: which window conf set
-- client: current window
function vsplitContainer(i, j, ratio)
    local i = math.min(i, #layoutData)
    local j = math.min(j, #layoutData[i])
    container = layoutData[i][j]

    print(i, j)
    x = container[1]
    y = container[2]
    width = container[3]
    height = container[4]

    container[3] = width * ratio
    newContainer = {x + container[3], y, width * (1-ratio), height}

    table.insert(layoutData[i], newContainer)
    action.arrangeThis(false)
end

function vsplitThisContainer(ratio)
    local i = math.max(math.min(info.thisTiledClientCount(), #layoutData), 1)
    local j = math.min(info.thisContainerPosition(), #layoutData[i])
    vsplitContainer(i, j, ratio)
end

function mergeContainer(i, j1, j2)
    if i > #layoutData then
        return
    end
    if math.max(j1, j2) > #layoutData[i] then
        return
    end

    local i = math.min(i, #layoutData)
    local j1 = math.min(j1, #layoutData[i])
    local j2 = math.min(j2, #layoutData[i])
    local container1 = layoutData[i][j1]
    local container2 = layoutData[i][j2]

    local x = math.min(container1[1], container2[1])
    local y = math.min(container1[2], container2[2])
    local width = math.max(container1[1] + container1[3],
                container2[1] + container2[3]) - x
    local height = math.max(container1[2] + container1[4],
                container2[2] + container2[4]) - y
    local newContainer = {x, y, width, height}

    layoutData[i][math.min(j1, j2)] = newContainer
    action.arrangeThis(false)
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
        layoutData[i][j] = {container[1], container[2] - n, container[3], container[4]}
    elseif d == Direction.BOTTOM then
        layoutData[i][j] = {container[1], container[2] + n, container[3], container[4]}
    elseif d == Direction.LEFT then
        layoutData[i][j] = {container[1] - n, container[2], container[3], container[4]}
    elseif d == Direction.RIGHT then
        layoutData[i][j] = {container[1] + n, container[2], container[3], container[4]}
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
        layoutData[i][j] = {container[0], container[2] - n, container[3],
        container[4] + n}
    elseif d == Direction.BOTTOM then
        layoutData[i][j] = {container[1], container[2], container[3],
        container[4] + n}
    elseif d == Direction.LEFT then
        layoutData[i][j] = {container[1] - n, container[2],
        container[3] + n, container[4]}
    elseif d == Direction.RIGHT then
        layoutData[i][j] = {container[1], container[2], container[3] + n,
        container[4]}
    end
end

function resizeThisContainer(n, d)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    resizeContainer(i, j, n, d)
    action.arrangeThis(false)
end

function resizeAll(i, j, n, d)
    local container = layoutData[i][j]
    for j2 = 1, #layoutData[i] do
        if j == j2 then
            resizeContainer(i, j, n, d)
        else
            -- resize container[i][j]?
            if d == Direction.TOP then
                resize = layoutData[i][j2][2] <= container[2]
            elseif d == Direction.BOTTOM then
                resize = layoutData[i][j2][2] >= container[2]
            elseif d == Direction.LEFT then
                resize = layoutData[i][j2][1] <= container[1]
                n = -n
            elseif d == Direction.RIGHT then
                resize = layoutData[i][j2][1] >= container[1]
            end

            if resize then
                resizeContainer(i, j2, -n, d)
                moveContainer(i, j2, n, d)
            end
        end
    end
end

function resizeMainAll(n, d)
    local i = math.max(math.min(info.thisTiledClientCount(), #layoutData), 1)
    resizeAll(i, 1, n, d)
    action.arrangeThis(false)
end

function resizeThisAll(n, d)
    local i = math.max(math.min(info.thisTiledClientCount(), #layoutData), 1)
    local j = math.min(info.thisContainerPosition(), #layoutData[i])
    print(i, j)
    resizeAll(i, j, n, d)
    action.arrangeThis(false)
end

function tile()
    layoutData = action.readOverlay("tile")
end

function monocle()
    layoutData = action.readOverlay("monocle")
end

function twoPane()
    layoutData = action.readOverlay("twoPane")
end

function loadLayout(layout)
    layoutData = action.readOverlay(layout)
end

function update(n)
    local i = math.max(math.min(#layoutData, n), 1)
    return layoutData[i]
end
