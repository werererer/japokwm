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

local function _copy(obj, target)
    local n = 0
  -- clear target table
    for k,v in pairs(target) do
        if type(v) == "table" then
            v.__del = true
            n = n + 1
        else
            target[k] = nil
        end
    end
  -- copy obj into target
    for k,v in pairs(obj) do
        if type(v) == "table" then
            local t = target[k]
            if t then
                t.__del = nil
                n = n - 1
            else
                t = {}
                target[k] = t
            end
            _copy(v, t)
        else
            target[k] = v
        end
    end
  -- clear no use sub table in target
    if n > 0 then
        for k,v in pairs(target) do
            if type(v) == "table" and v.__del then
                target[k] = nil
            end
        end
    end
end

local function deepcopy(obj, target)
    target = target or {}
    _copy(obj, target)
    return target
end

-- set: which window conf set
-- client: current window
function Move_container(container, n, d)
    local con = container
    if d == Direction.TOP then
        con[Y] = con[Y] + n
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
        con[X] = con[X] - n
        con[WIDTH] = con[WIDTH] + n
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
    local con = deepcopy(container)
    con = Move_container(con, nmove, d)
    con = Resize_container(con, nresize, d)
    return con
end

-- n: number
-- d: amount of digits
function Floor(n, d)
    if (n < 1e-10) then return 0 end
    local power = 10^d
    return math.floor(n * power) / power
end

function Is_approx_equal(a, b)
    return math.abs(a - b) < 0.001
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

function Is_container_right_to(ref_con, con)
    local right = con[X] > ref_con[X] + ref_con[WIDTH]
    right = right or Is_approx_equal(con[X], ref_con[X] + ref_con[WIDTH])
    return right
end

function Is_container_left_to(ref_con, con)
    local left = con[X] + con[WIDTH] < ref_con[X]
    left = left or Is_approx_equal(con[X] + con[WIDTH], ref_con[X])
    return left
end

function Is_container_over(ref_con, con)
    local over = con[Y] + con[HEIGHT] < ref_con[Y]
    over = over or Is_approx_equal(con[Y] + con[HEIGHT], ref_con[Y])
    return over
end

function Is_container_under(ref_con, con)
    local under = con[Y] > ref_con[Y] + ref_con[HEIGHT]
    under = under or Is_approx_equal(con[Y], ref_con[Y] + ref_con[HEIGHT])
    return under
end

function Does_container_not_intersect_with(ref_con, con)
    return Is_container_left_to(ref_con, con)
    or Is_container_right_to(ref_con, con)
    or Is_container_under(ref_con, con)
    or Is_container_over(ref_con, con)
end

-- returns whether container2 is affected
function Is_affected_by_resize_of(container, container2, d)
    local resize = false

    if d == Direction.TOP then
        local right = Is_container_right_to(container, container2)
        local left = Is_container_left_to(container, container2)
        local container_is_higher = Is_container_over(container, container2)

        resize = container_is_higher and not (left or right)
    elseif d == Direction.BOTTOM then
        local right = Is_container_right_to(container, container2)
        local left = Is_container_left_to(container, container2)
        local container_is_lower = Is_container_under(container, container2)

        resize = container_is_lower and not (left or right)
    elseif d == Direction.LEFT then
        local over = Is_container_over(container, container2)
        local under = Is_container_under(container, container2)
        local container_is_left = Is_container_left_to(container, container2)

        resize = container_is_left and not (over or under)
    elseif d == Direction.RIGHT then
        local over = Is_container_over(container, container2)
        local under = Is_container_under(container, container2)
        local container_is_right = Is_container_right_to(container, container2)

        resize = container_is_right and not (over or under)
    end

    return resize
end

function Is_equally_affected_by_resize_of(container, container2, d)
    local resize = false
    if d == Direction.TOP then
        resize = Is_approx_equal(container2[Y], container[Y])
    elseif d == Direction.BOTTOM then
        -- if equal (because of rounding issues it has to be written like that)
        resize = Is_approx_equal(container2[Y] + container2[HEIGHT], container[Y] + container[HEIGHT])
    elseif d == Direction.LEFT then
        resize = Is_approx_equal(container2[X], container[X])
    elseif d == Direction.RIGHT then
        resize = Is_approx_equal(container2[X] + container2[WIDTH], container[X] + container[WIDTH])
    end
    return resize
end

-- finds containers that are affected by the container at i,j
function Get_resize_affected_containers(i, j, d)
    local container = Layout_data[i][j]
    local list = {}

    for j2 = 1, #Layout_data[i] do
        local con = Layout_data[i][j2]
        local alt_con = Get_alternative_container(container, d)

        if j ~= j2 then
            if Is_affected_by_resize_of(container, con, d) then
                -- convert relative to absolute box
                local ret_con = {con[X], con[Y], con[WIDTH], con[HEIGHT], i, j2}
                ret_con[X] = (ret_con[X]-alt_con[X])/alt_con[WIDTH]
                ret_con[Y] = (ret_con[Y]-alt_con[Y])/alt_con[HEIGHT]
                ret_con[WIDTH] = ret_con[WIDTH]/alt_con[WIDTH]
                ret_con[HEIGHT] = ret_con[HEIGHT]/alt_con[HEIGHT]
                table.insert(list, ret_con)
            end
        end
    end
    return list
end

-- TODO: reduce function size
-- finds containers that are affected by the container at i,j
function Get_resize_affected_main_containers(i, j, d)
    local container = Layout_data[i][j]
    local list = {}
    local main_con = Get_main_container(container, d)

    for j2 = 1, #Layout_data[i] do
        local con = Layout_data[i][j2]

        if j ~= j2 then
            if Is_equally_affected_by_resize_of(con, container, d) then
                -- convert relative to absolute box
                local ret_con = {con[X], con[Y], con[WIDTH], con[HEIGHT], i, j2}
                ret_con[X] = (ret_con[X]-main_con[X])/main_con[WIDTH]
                ret_con[Y] = (ret_con[Y]-main_con[Y])/main_con[HEIGHT]
                ret_con[WIDTH] = ret_con[WIDTH]/main_con[WIDTH]
                ret_con[HEIGHT] = ret_con[HEIGHT]/main_con[HEIGHT]
                table.insert(list, ret_con)
            end
        end
    end
    return list
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
    local alt_con = {0, 0, 1, 1}
    if d == Direction.TOP then
        alt_con[Y] = 0
        alt_con[HEIGHT] = container[Y]
    elseif d == Direction.BOTTOM then
        alt_con[Y] = container[Y] + container[HEIGHT]
        alt_con[HEIGHT] = 1 - alt_con[Y]
    elseif d == Direction.LEFT then
        alt_con[X] = 0
        alt_con[WIDTH] = container[X]
    elseif d == Direction.RIGHT then
        alt_con[X] = container[X] + container[WIDTH]
        alt_con[WIDTH] = 1 - alt_con[X]
    end
    return alt_con
end

function Get_main_container(container, d)
    local con = {container[X], container[Y], container[WIDTH], container[HEIGHT]}
    -- if d == Direction.TOP then
    --     con[X] = 0
    --     con[WIDTH] = 1
    -- elseif d == Direction.BOTTOM then
    --     con[X] = 0
    --     con[WIDTH] = 1
    -- elseif d == Direction.LEFT then
    --     con[Y] = 0
    --     con[HEIGHT] = 1
    -- elseif d == Direction.RIGHT then
    --     con[Y] = 0
    --     con[HEIGHT] = 1
    -- end
    return con
end

-- put all directions into a list
function Get_directions(d)
    list = {}
    if d >= 8 then
        table.insert(list, 8)
        d = d - 8
    end
    if d >= 4 then
        table.insert(list, 4)
        d = d - 4
    end
    if d >= 2 then
        table.insert(list, 2)
        d = d - 2
    end
    if d >= 1 then
        table.insert(list, 1)
        d = d - 1
    end
    return list
end

function Resize_all(i, j, n, d)
    local directions = Get_directions(d)
    local container = Layout_data[i][j]

    local lock = false
    -- check
    for x = 1,#directions do
        local dir = directions[x]
        -- local resize_main_containers = Get_resize_affected_main_containers(i, j, dir)
        local resize_containers = Get_resize_affected_containers(i, j, dir)
        local main_con = Move_resize(container, 0, n, dir)
        local alt_con = Get_alternative_container(main_con, dir)

        lock = lock or main_con[WIDTH] < Min_main_width or main_con[HEIGHT] < Min_main_height
        lock = lock or main_con[WIDTH] > Max_main_width
        lock = lock or main_con[HEIGHT] > Max_main_height
        local con = deepcopy(Layout_data)
        for k = 1,#resize_containers do
            local li = resize_containers[k][5]
            local lj = resize_containers[k][6]

            con[li][lj][X] = alt_con[X] + (resize_containers[k][X] * alt_con[WIDTH])
            con[li][lj][Y] = alt_con[Y] + (resize_containers[k][Y] * alt_con[HEIGHT])
            con[li][lj][WIDTH] = resize_containers[k][WIDTH] * alt_con[WIDTH]
            con[li][lj][HEIGHT] = resize_containers[k][HEIGHT] * alt_con[HEIGHT]

            local c = con[li][lj]
            lock = lock or c[WIDTH] < Min_width or c[HEIGHT] < Min_height
            lock = lock or c[WIDTH] > Max_width
            lock = lock or c[HEIGHT] > Max_height
        end
    end

    if lock then
      return
    end

    -- apply
    for x = 1,#directions do
        local dir = directions[x]
        local resize_main_containers = Get_resize_affected_main_containers(i, j, dir)
        local resize_containers = Get_resize_affected_containers(i, j, dir)
        local main_con = Move_resize(container, 0, n, dir)
        local alt_con = Get_alternative_container(main_con, dir)

        for k = 1,#resize_containers do
            local li = resize_containers[k][5]
            local lj = resize_containers[k][6]

            Layout_data[li][lj][X] = alt_con[X] + (resize_containers[k][X] * alt_con[WIDTH])
            Layout_data[li][lj][Y] = alt_con[Y] + (resize_containers[k][Y] * alt_con[HEIGHT])
            Layout_data[li][lj][WIDTH] = resize_containers[k][WIDTH] * alt_con[WIDTH]
            Layout_data[li][lj][HEIGHT] = resize_containers[k][HEIGHT] * alt_con[HEIGHT]
        end

        Layout_data[i][j][X] = main_con[X]
        Layout_data[i][j][Y] = main_con[Y]
        Layout_data[i][j][WIDTH] = main_con[WIDTH]
        Layout_data[i][j][HEIGHT] = main_con[HEIGHT]
        for k = 1,#resize_main_containers do
            local li = resize_main_containers[k][5]
            local lj = resize_main_containers[k][6]
            local lock = false
            Layout_data[li][lj],lock = Move_resize(Layout_data[li][lj], 0, n, dir)
            if lock then
                print("locked")
            end
        end
    end
end

function Resize_main_all(n, d)
    local i = math.max(math.min(info.this_tiled_client_count(), #Layout_data), 1)

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
