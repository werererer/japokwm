Direction = {
    TOP = 1,
    BOTTOM = 2,
    LEFT = 4,
    RIGHT = 8,
}

local X<const> = 1
local Y<const> = 2
local WIDTH<const> = 3
local HEIGHT<const> = 4

Resize_direction = Direction.BOTTOM

-- current layout_data
-- layout_data = layout item list
-- container list = layout item
-- list of 4 floats = container
Layout_data = {
    {
        {0.3, 0, 0.4, 1},
    },
}

Master_layout_data = {
    {
        {0, 0, 0.8, 1},
    },
    {
        {0, 0, 1, 0.5},
        {0, 0.5, 1, 0.5},
    },
}

Box_data = {
    {1},
    {2, 3, 4, 5},
}

-- set: which window conf set
-- client: current window
function Move_container(container, n, d)
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

function Move_this_container(n, d)
    local i = math.max(math.min(action.this_tiled_client_count(), #Layout_data), 1)
    local j = math.min(info.this_container_position(), #Layout_data[i])
    local container = Layout_data[i][j]
    Layout_data[i][j] = Move_container(container, n, d)
    action.arrange_this(false)
end

function Resize_container(container, n, d)
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

function Resize_this_container(n, d)
    local i = math.max(math.min(action.this_tiled_client_count(), #Layout_data), 1)
    local j = math.min(action.client_pos(), #Layout_data[i])
    Layout_data[i][j] = Resize_container(Layout_data[i][j], n, d)
    action.arrange_this(false)
end

function Move_resize(container, nmove, nresize, d)
    local con = container
    con = Move_container(con, nmove, d)
    con = Resize_container(con, nresize, d)
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
function Get_alternative_container(container, d)
    local alt = {0, 0, 1, 1}
    if d == Direction.TOP then
        alt[Y] = 0
        alt[HEIGHT] = container[Y]
    elseif d == Direction.BOTTOM then
        alt[Y] = container[Y] + container[HEIGHT]
        alt[HEIGHT] = 1 - (container[Y] + container[HEIGHT])
    elseif d == Direction.LEFT then
        alt[X] = 0
        alt[WIDTH] = container[X]
    elseif d == Direction.RIGHT then
        alt[X] = container[X] + container[WIDTH]
        alt[WIDTH] = 1 - alt[X]
    end
    return alt
end

function Get_current_container(container, d)
    local alt = {0, 0, 1, 1}
    if d == Direction.TOP then
        alt[Y] = 0
        alt[HEIGHT] = container[Y]
    elseif d == Direction.BOTTOM then
        alt[Y] = container[Y] + container[HEIGHT]
        alt[HEIGHT] = 1 - (container[Y] + container[HEIGHT])
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
function Is_affected_by_resize_of(container, container2, d)
    local resize = false
    if d == Direction.TOP then
        resize = container2[Y] > container[Y]
        local left = container2[X] >= container[X] + container[WIDTH]
        local right = container2[X] + container2[WIDTH] <= container[X]
        resize = resize and not (left or right)
    elseif d == Direction.BOTTOM then
        resize = container2[Y] < container[Y]
        local left = container2[X] >= container[X] + container[WIDTH]
        local right = container2[X] + container2[WIDTH] <= container[X]
        resize = resize and not (left or right)
    elseif d == Direction.LEFT then
        resize = container2[X] > container[X]
        local over = container2[Y] >= container[Y] + container[HEIGHT]
        local under = container2[Y] + container2[HEIGHT] <= container[Y]
        resize = resize and not (over or under)
    elseif d == Direction.RIGHT then
        resize = container2[X] < container[X]
        local over = container2[Y] >= container[Y] + container[HEIGHT]
        local under = container2[Y] + container2[HEIGHT] <= container[Y]
        resize = resize and not (over or under)
    end
    return resize
end

function Is_equally_affected_by_resize_of(container, container2, d)
    local resize = false
    if d == Direction.TOP then
        resize = container2[Y] < container[Y] + container[HEIGHT]
    elseif d == Direction.BOTTOM then
        resize = container2[Y] < container[Y] + container[HEIGHT]
    elseif d == Direction.LEFT then
        resize = container2[X] > container[X]
        local over = container2[Y] >= container[Y] + container[HEIGHT]
        local under = container2[Y] + container2[HEIGHT] <= container[Y]
        resize = resize and not (over or under)
    elseif d == Direction.RIGHT then
        resize = container2[X] < container[X]
        local over = container2[Y] >= container[Y] + container[HEIGHT]
        local under = container2[Y] + container2[HEIGHT] <= container[Y]
        resize = resize and not (over or under)
    end
    return resize
end

-- finds containers that are effected by the container at i,j
function Get_resize_effected_containers(i, j, d)
    local container = Layout_data[i][j]
    local list = {}
    local alt_con = Get_alternative_container(container, d)

    for j2 = 1, #Layout_data[i] do
        local con = Layout_data[i][j2]

        if j ~= j2 then
            if Is_affected_by_resize_of(con, container, d) then
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
--
-- finds containers that are effected by the container at i,j
function Get_resize_effected_main_containers(i, j, d)
    local container = Layout_data[i][j]
    local list = {}
    local main_con = Get_main_container(container, d)

    for j2 = 1, #Layout_data[i] do
        local con = Layout_data[i][j2]

        if j ~= j2 then
            if Is_equally_affected_by_resize_of(con, container, d) then
                print("S")
                -- convert relative to absolute box
                local d = {con[X], con[Y], con[WIDTH], con[HEIGHT], i, j2}
                print(con[X], con[Y], con[WIDTH], con[HEIGHT])
                print(main_con[X], main_con[Y], main_con[WIDTH], main_con[HEIGHT])
                d[X] = (d[X]-main_con[X])/main_con[WIDTH]
                d[Y] = (d[Y]-main_con[Y])/main_con[HEIGHT]
                d[WIDTH] = d[WIDTH]/main_con[WIDTH]
                d[HEIGHT] = d[HEIGHT]/main_con[HEIGHT]
                print(d[X], d[Y], d[WIDTH], d[HEIGHT])
                table.insert(list, d)
                print("E")
            end
        end
    end
    return list
end


function Get_main_container(container, d)
    local con = {container[X], container[Y], container[WIDTH], container[HEIGHT]}
    if d == Direction.TOP then
        con[X] = 0
        con[WIDTH] = 1
    elseif d == Direction.BOTTOM then
        con[X] = 0
        con[WIDTH] = 1
    elseif d == Direction.LEFT then
        con[Y] = 0
        con[HEIGHT] = 1
    elseif d == Direction.RIGHT then
        con[Y] = 0
        con[HEIGHT] = 1
    end
    return con
end

function Resize_all(i, j, n, d)
    print("resize_all")
    local container = Layout_data[i][j]
    local resize_containers = Get_resize_effected_containers(i, j, d)
    local resize_main_containers = Get_resize_effected_main_containers(i, j, d)
    Layout_data[i][j] = Move_resize(container, 0, n, d)
    local main_con = Get_main_container(container, d)
    local alt_con = Get_alternative_container(container, d)

    for k = 1,#resize_main_containers do
        local li = resize_main_containers[k][5]
        local lj = resize_main_containers[k][6]

        Layout_data[li][lj][X] = main_con[X] + (resize_main_containers[k][X] * main_con[WIDTH])
        Layout_data[li][lj][Y] = main_con[Y] + (resize_main_containers[k][Y] * main_con[HEIGHT])
        Layout_data[li][lj][WIDTH] = resize_main_containers[k][WIDTH] * main_con[WIDTH]
        Layout_data[li][lj][HEIGHT] = resize_main_containers[k][HEIGHT] * main_con[HEIGHT]
    end

    for k = 1,#resize_containers do
        local li = resize_containers[k][5]
        local lj = resize_containers[k][6]

        Layout_data[li][lj][X] = alt_con[X] + (resize_containers[k][X] * alt_con[WIDTH])
        Layout_data[li][lj][Y] = alt_con[Y] + (resize_containers[k][Y] * alt_con[HEIGHT])
        Layout_data[li][lj][WIDTH] = resize_containers[k][WIDTH] * alt_con[WIDTH]
        Layout_data[li][lj][HEIGHT] = resize_containers[k][HEIGHT] * alt_con[HEIGHT]
    end
    print("end")
end

function Resize_main_all(n)
    local d = Resize_direction
    local i = math.max(math.min(info.this_tiled_client_count(), #Layout_data), 1)
    local max = math.max(#Layout_data, 1)

    for g=1,#Box_data do
        for h=1,#Box_data[g] do
            if i == Box_data[g][h] then
                for j=1,#Box_data[g] do
                    Resize_all(Box_data[g][j], 1, n, d)
                    action.arrange_this(false)
                end
                break
            end
        end
    end
end

function Resize_this_all(n, d)
    local i = math.max(math.min(info.this_tiled_client_count(), #Layout_data), 1)
    local j = math.min(info.this_container_position(), #Layout_data[i])
    Resize_all(i, j, n, d)
    action.arrange_this(false)
end

function Load_layout(layout_name)
    local layout, master_layout, boxes
    action.load_layout(layout_name)
    if layout then
        Layout_data = layout
    end
    if master_layout then
        Master_layout_data = master_layout
    end
    if boxes then
        Box_data = boxes
    end
    action.arrange_this(false);
end

-- TODO: improve function name which doesn't representing what it does
function Update_layout(n)
    local i = math.max(math.min(#Layout_data, n), 1)
    return Layout_data[i]
end

-- TODO: improve function name not representing what it does
function Update_nmaster(n)
    local i = math.max(math.min(#Master_layout_data, n), 1)
    return Master_layout_data[i]
end
