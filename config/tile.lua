
-- function add(box)
--     ccall((:add_box, core_path), Cvoid, (Cint, Cint, Cint, Cint), 3, 3, 4, 5)
-- end

-- function del()
--     ccall((:del, core_path), Cvoid, (Cint,), 3)
-- end

Direction = {
    TOP = 1,
    BOTTOM = 2,
    LEFT = 3,
    RIGHT = 4,
}

local X<const> = 1
local Y<const> = 2
local WIDTH<const> = 3
local HEIGHT<const> = 4

-- current layout_data
-- layout_data = 2 layouts where layout[1] = nmaster layout and layout[2] =
-- current layout
-- layout = layout item list
-- container list = layout item
-- list of 4 floats = container
layout_data = {
    {
        {
            {0.3, 0, 0.4, 1},
        },
    }
}

master_layout_data = {
    {
        {0, 0, 0.8, 1},
    },
    {
        {0, 0, 1, 0.5},
        {0, 0.5, 1, 0.5},
    },
}

box_data = {
    {1},
    {2, 3, 4, 5},
}

-- transformations are saved and are reapplied when a new thing loads
transformations = {
    {action, con1, con2, ratio, direction}
}

-- set: which window conf set
-- client: current window
function split_container(i, j, ratio)
    local i = math.min(i, #layout_data)
    local j = math.min(j, #layout_data[i])
    container = layout_data[i][j]

    print(i, j)
    x = container[X]
    y = container[Y]
    width = container[WIDTH]
    height = container[HEIGHT]

    container[HEIGHT] = height * ratio
    new_container = {x, y + container[HEIGHT], width, height * (1-ratio)}

    table.insert(layout_data[i], new_container)
    action.arrange_this(false)
end

function split_this_container(ratio)
    local i = math.max(math.min(info.this_tiled_client_count(), #layout_data), 1)
    local j = math.min(info.this_container_position(), #layout_data[i])
    split_container(i, j, ratio)
end

-- set: which window conf set
-- client: current window
function vsplit_container(i, j, ratio)
    local i = math.min(i, #layout_data)
    local j = math.min(j, #layout_data[i])
    container = layout_data[i][j]

    print(i, j)
    x = container[X]
    y = container[Y]
    width = container[WIDTH]
    height = container[HEIGHT]

    container[WIDTH] = width * ratio
    new_container = {x + container[WIDTH], y, width * (1-ratio), height}

    table.insert(layout_data[i], new_container)
    action.arrange_this(false)
end

function vsplit_this_container(ratio)
    local i = math.max(math.min(info.this_tiled_client_count(), #layout_data), 1)
    local j = math.min(info.this_container_position(), #layout_data[i])
    vsplit_container(i, j, ratio)
end

function merge_container(i, j1, j2)
    if i > #layout_data then
        return
    end
    if math.max(j1, j2) > #layout_data[i] then
        return
    end

    local i = math.min(i, #layout_data)
    local j1 = math.min(j1, #layout_data[i])
    local j2 = math.min(j2, #layout_data[i])
    local container1 = layout_data[i][j1]
    local container2 = layout_data[i][j2]

    local x = math.min(container1[X], container2[X])
    local y = math.min(container1[Y], container2[Y])
    local width = math.max(container1[X] + container1[WIDTH],
                container2[X] + container2[WIDTH]) - x
    local height = math.max(container1[Y] + container1[HEIGHT],
                container2[Y] + container2[HEIGHT]) - y
    local new_container = {x, y, width, height}

    layout_data[i][math.min(j1, j2)] = new_container
    action.arrange_this(false)
end

function move_container(container, n, d)
    local con = container
    if d == Direction.TOP then
        con[Y] = con[Y] - n
    elseif d == Direction.BOTTOM then
        con[Y] = con[Y] + n
    elseif d == Direction.LEFT then
        con[X] = con[X] + n
    elseif d == Direction.RIGHT then
        con[X] = con[X] + n
    end
    return con
end

function move_this_container(n, d)
    local i = max(min(this_tiled_client_count(), length(layout_data)), 1)
    local j = min(client_pos(), length(layout_data[i]))
    local container = layout_data[i][j]
    layout_data[i][j] = move_container(container, n, d)
    arrange_this(false)
end

function resize_container(container, n, d)
    local con = container
    if d == Direction.TOP then
        con[Y] = con[Y] - n
        con[HEIGHT] = con[HEIGHT] + n
    elseif d == Direction.BOTTOM then
        con[HEIGHT] = con[HEIGHT] + n
    elseif d == Direction.LEFT then
        con[X] = con[X] + n
        con[WIDTH] = con[WIDTH] - n
    elseif d == Direction.RIGHT then
        con[WIDTH] = con[WIDTH] + n
    end
    return con
end

function resize_this_container(n, d)
    local i = max(min(this_tiled_client_count(), length(layout_data)), 1)
    local j = min(client_pos(), length(layout_data[i]))
    layout_data[i][j] = resize_container(layout_data[i][j], n, d)
    action.arrange_this(false)
end

function move_resize(container, nmove, nresize, d)
    local con = container
    con = move_container(con, nmove, d)
    con = resize_container(con, nresize, d)
    return con
end

-- if d == Direction.LEFT then "raytrace" to the left like that and return the
-- geometry of that area
-- +--------------------------+
-- |< - - - - +---------+     |
-- ||         |         |     |
-- ||    a    |    o    |     |
-- ||         |         |     |
-- |< - - - - +---------+     |
-- +--------------------------+
-- where w is the original window and a is the alternative window
function get_alternative_container(container, d)
    local alt = {0, 0, 1, 1}
    if d == Direction.TOP then
        alt[Y] = 0
        alt[HEIGHT] = container[Y]
    elseif d == Direction.BOTTOM then
        alt[Y] = container[Y] + container[HEIGHT]
        alt[HEIGHT] = 1 - container[Y]
    elseif d == Direction.LEFT then
        alt[X] = 0
        alt[WIDTH] = container[X]
    elseif d == Direction.RIGHT then
        alt[X] = container[X] + container[WIDTH]
        alt[WIDTH] = 1 - alt[X]
    end
    return alt
end

-- returns whether container2 is effected
function is_affected_by_resize_of(container, container2, d)
    local resize = false
    if d == Direction.TOP then
        resize = container2[Y] < container[Y]
        local left = container2[X] >= container[X] + container[WIDTH]
        local right = container2[X] + container2[WIDTH] <= container[X]
        resize = resize and not (left or right)
    elseif d == Direction.BOTTOM then
        resize = container2[Y] > container[Y]
        local left = container2[X] >= container[X] + container[WIDTH]
        local right = container2[X] + container2[WIDTH] <= container[X]
        resize = resize and not (left or right)
    elseif d == Direction.LEFT then
        resize = container2[X] < container[X]
        local over = container2[Y] >= container[Y] + container[HEIGHT]
        local under = container2[Y] + container2[HEIGHT] <= container[Y]
        resize = resize and not (over or under)
    elseif d == Direction.RIGHT then
        resize = container[X] > container2[X]
        local over = container2[Y] >= container[Y] + container[HEIGHT]
        local under = container2[Y] + container2[HEIGHT] <= container[Y]
        resize = resize and not (over or under)
    end
    return resize
end

-- finds containers that are effected by the container at i,j
function get_resize_effected_containers(i, j, d)
    local container = layout_data[i][j]
    local list = {}
    local alt_con = get_alternative_container(container, d)

    for j2 = 1, #layout_data[i] do
        local con = layout_data[i][j2]
        local resize = false

        if j ~= j2 then
            if is_affected_by_resize_of(con, container, d) then
                -- convert relative to absolute box
                local d = {con[X], con[Y], con[WIDTH], con[HEIGHT], i, j2}
                d[X] = (d[X]-alt_con[X])/alt_con[WIDTH]
                d[Y] = (d[Y]-alt_con[Y])/alt_con[HEIGHT]
                d[WIDTH] = d[WIDTH]/alt_con[WIDTH]
                d[HEIGHT] = d[HEIGHT]/alt_con[HEIGHT]
                table.insert(list, d)
            end
        end
    end
    return list
end


function resize_all(i, j, n, d)
    local container = layout_data[i][j]
    local resize_containers = get_resize_effected_containers(i, j, d)
    layout_data[i][j] = move_resize(container, 0, n, d)
    local alt_con = get_alternative_container(container, d)

    for k = 1,#resize_containers do
        local li = resize_containers[k][5]
        local lj = resize_containers[k][6]
        layout_data[li][lj][X] = alt_con[X] + (resize_containers[k][X] * alt_con[WIDTH])
        layout_data[li][lj][Y] = alt_con[Y] + (resize_containers[k][Y] * alt_con[HEIGHT])
        layout_data[li][lj][WIDTH] = resize_containers[k][WIDTH] * alt_con[WIDTH]
        layout_data[li][lj][HEIGHT] = resize_containers[k][HEIGHT] * alt_con[HEIGHT]
    end
end

function resize_main_all(n, d)
    local i = math.max(math.min(info.this_tiled_client_count(), #layout_data), 1)
    local max = math.max(#layout_data, 1)

    for g=1,#box_data do
        for h=1,#box_data[g] do
            if i == box_data[g][h] then
                for j=1,#box_data[g] do
                    resize_all(box_data[g][j], 1, n, d)
                    action.arrange_this(false)
                end
                break
            end
        end
    end
end

function resize_this_all(n, d)
    local i = math.max(math.min(info.this_tiled_client_count(), #layout_data), 1)
    local j = math.min(info.this_container_position(), #layout_data[i])
    resize_all(i, j, n, d)
    action.arrange_this(false)
end

function tile()
    local layout, master_layout, boxes
    print("tile1")
    layout, master_layout, boxes = action.read_layout("tile")
    print("tile2")
    if layout then
        layout_data = layout
    end
    if master_layout then
        master_layout_data = master_layout
    end
    if boxes then
        box_data = boxes
    end
    print(#layout_data, #master_layout_data)
end

function monocle()
    layout_data, master_layout_data = action.read_layout("monocle")
end

function two_pane()
    layout_data, master_layout_data = action.read_layout("two_pane")
end

function load_layout(layout)
    print("load layout")
    layout_data, master_layout_data = action.read_layout(layout)
end

-- TODO: improve function name not representing what it does
function update_layout(n)
    local i = math.max(math.min(#layout_data, n), 1)
    return layout_data[i]
end

-- TODO: improve function name not representing what it does
function update_nmaster(n)
    local i = math.max(math.min(#master_layout_data, n), 1)
    return master_layout_data[i]
end
