
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
    local startx, starty, width, height = getStartv(i, j, d)
    for j2 = 1, #layoutData[i] do
        local con = layoutData[i][j2]
        local resize = false

        if j == j2 then
        else
            -- resize container[i][j]?
            if d == Direction.TOP then
                resize = con[2] < container[2]
                local left = con[1] >= container[1] + container[3]
                local right = con[1] + con[3] <= container[1]
                resize = resize and not (over or under)
            elseif d == Direction.BOTTOM then
                resize = con[2] > container[2]
                local left = con[1] >= container[1] + container[3]
                local right = con[1] + con[3] <= container[1]
                resize = resize and not (left or right)
            elseif d == Direction.LEFT then
                resize = con[1] < container[1]
                over = con[2] >= container[2] + container[4]
                under = con[2] + con[4] <= container[2]
                resize = resize and not (over or under)
            elseif d == Direction.RIGHT then
                resize = con[1] > container[1]
                over = con[2] >= container[2] + container[4]
                under = con[2] + con[4] <= container[2]
                resize = resize and not (over or under)
            end

            if resize then
                local d = {con[1], con[2], con[3], con[4], i, j2}
                d[1] = (d[1]-startx)/width
                d[2] = (d[2]-starty)/height
                d[3] = d[3]/width
                d[4] = d[4]/height
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

function getStartv(i, j, d)
    local startx, starty, width, height = 0, 0, 1, 1
    if d == Direction.TOP then
        starty = 0
        height = layoutData[i][j][2]
    elseif d == Direction.BOTTOM then
        starty = layoutData[i][j][2] + layoutData[i][j][4]
        height = 1 - starty
    elseif d == Direction.LEFT then
        startx = 0
        width = layoutData[i][j][1]
    elseif d == Direction.RIGHT then
        startx = layoutData[i][j][1] + layoutData[i][j][3]
        width = 1 - startx
    end
    return startx, starty, width, height
end

function resizeAll(i, j, n, d)
    local container = layoutData[i][j]
    local g = count(i, j, d)
    layoutData[i][j] = moveResize(container, 0, n, d)
    local startx, starty, width, height = getStartv(i, j, d)

    for k = 1,#g do
        local li = g[k][5]
        local lj = g[k][6]
        layoutData[li][lj][1] = startx + (g[k][1] * width)
        layoutData[li][lj][2] = starty + (g[k][2] * height)
        layoutData[li][lj][3] = g[k][3] * width
        layoutData[li][lj][4] = g[k][4] * height
    end
    -- layoutData[i][j2] = moveResize(con, nmove, -n/k, d)

    -- for j2 = startv,endv,step do
    --     local con = layoutData[i][j2]
    --     local resize = false
    --     local nresize = false

    --     if j2 == j then
    --     else
    --         -- resize container[i][j]?
    --         if d == Direction.TOP then
    --             resize = con[2] <= container[2]
    --         elseif d == Direction.BOTTOM then
    --             resize = con[2] >= container[2]
    --         elseif d == Direction.LEFT then
    --             resize = container[1] > con[1]
    --         elseif d == Direction.RIGHT then
    --             resize = container[1] < con[1]
    --         end

    --         if resize then
    --             -- print("if resize")
    --             -- -- k is the number of windows right to this (this
    --             -- -- window included)
    --             -- -- layoutData[i][j2] = moveResize(con, nmove, -n/k, d)
    --             -- layoutData[i][j2] = moveResize(con, nmove, -n/k, d)
    --             -- -- nmove is the move of the next window right to
    --             -- -- this
    --             -- nmove = getNextMove(nmove, -n/k)
    --         end
    --     end
    -- end
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
