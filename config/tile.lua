
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

function moveContainer(container, n, d)
    local con = container
    if d == Direction.TOP then
        con = {con[1], con[2] - n, con[3], con[4]}
    elseif d == Direction.BOTTOM then
        con = {con[1], con[2] + n, con[3], con[4]}
    elseif d == Direction.LEFT then
        con = {con[1] + n, con[2], con[3], con[4]}
    elseif d == Direction.RIGHT then
        con = {con[1] + n, con[2], con[3], con[4]}
    end
    return con
end

function moveThisContainer(n, d)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    layoutData[i][j] = moveContainer(layoutData[i][j], n, d)
    arrangeThis(false)
end

function resizeContainer(container, n, d)
    local con = container
    if d == Direction.TOP then
        con = {con[1], con[2] - n, con[3],
        con[4] + n}
    elseif d == Direction.BOTTOM then
        con = {con[1], con[2], con[3],
        con[4] + n}
    elseif d == Direction.LEFT then
        con = {con[1] + n, con[2],
        con[3] - n, con[4]}
    elseif d == Direction.RIGHT then
        con = {con[1], con[2], con[3] + n,
        con[4]}
    end
    return con
end

function resizeThisContainer(n, d)
    local i = max(min(thisTiledClientCount(), length(layoutData)), 1)
    local j = min(clientPos(), length(layoutData[i]))
    layoutData[i][j] = resizeContainer(layoutData[i][j], n, d)
    action.arrangeThis(false)
end

function count(i, j, d)
    local container = layoutData[i][j]
    local list = {}
    -- 100%
    local startv = container[1] + container[3]
    local full = 1 - startv
    for j2 = 1, #layoutData[i] do
        local con = layoutData[i][j2]
        local resize = false

        if j == j2 then
        else
            -- resize container[i][j]?
            if d == Direction.TOP then
                resize = con[2] <= container[2]
            elseif d == Direction.BOTTOM then
                resize = con[2] >= container[2]
            elseif d == Direction.LEFT then
                resize = container[1] > con[1]
            elseif d == Direction.RIGHT then
                resize = container[1] < con[1]
            end

            if resize then
                startv = container[1] + container[3]
                local d = {con[1]-startv, con[2], con[3], con[4], i, j2}
                print("x/x2:", con[1], startv, full)
                d[1] = d[1]/full
                d[2] = d[2]
                d[3] = d[3]/full
                d[4] = d[4]
                print(d[1], d[2], d[3], d[4])
                table.insert(list, d)
            end
        end
    end
    return list
end

function getNextMove(nmove, nresize)
    return nmove + nresize
end

function moveResize(container, nmove, nresize, d)
    local con = container
    print("mr:", nmove, nresize)
    con = moveContainer(con, nmove, d)
    con = resizeContainer(con, nresize, d)
    return con
end

function resizeAll(i, j, n, d)
    local container = layoutData[i][j]
    local nmove = 0

    local g = count(i, j, d)
    layoutData[i][j] = moveResize(container, 0, n, d)
    nmove = getNextMove(0, n)
    print(g[1], g[2], g[3], g[4])

    local k = #g
    if k < 1 then
        k = 1
    end
    local startv = 1
    local endv = #layoutData[i]
    local step = 1

    for k = 1,#g do
        local li = g[k][5]
        local lj = g[k][6]
        local startv = layoutData[i][j][1] + layoutData[i][j][3]
        local newWidth = 1 - startv
        print("start", li, lj, startv, newWidth)
        layoutData[li][lj][1] = startv + (g[k][1] * newWidth)
        print("continue")
        layoutData[li][lj][2] = g[k][2]
        layoutData[li][lj][3] = g[k][3] * newWidth
        layoutData[li][lj][4] = g[k][4]
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
