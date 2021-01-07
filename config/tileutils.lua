Direction = {
    TOP = 1,
    BOTTOM = 2,
    LEFT = 4,
    RIGHT = 8,
}

Original_layout_data = {}

local X<const> = 1
local Y<const> = 2
local WIDTH<const> = 3
local HEIGHT<const> = 4

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

function Deepcopy(obj, target)
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

function Is_resize_locked(i, j, n, directions)
    local container = Layout_data[i][j]
    local lock = false

    for x = 1,#directions do
        local dir = directions[x]
        local resize_containers = Get_resize_affected_containers(i, j, dir, Get_alternative_container, Is_affected_by_resize_of)
        local main_con = Move_resize(container, 0, n, dir)
        local alt_con = Get_alternative_container(main_con, dir)

        lock = lock or main_con[WIDTH] < Min_main_width or main_con[HEIGHT] < Min_main_height
        lock = lock or main_con[WIDTH] > Max_main_width
        lock = lock or main_con[HEIGHT] > Max_main_height
        local con = Deepcopy(Layout_data)
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
    return lock
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

function Move_resize(container, nmove, nresize, d)
    local con = Deepcopy(container)
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

-- put all directions into a list
function Get_directions(d)
    local list = {}
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

function Get_edge_container(container, d)
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
        con[X] = container[X] + container[WIDTH]
        con[Y] = container[Y] + container[HEIGHT]
        con[WIDTH] = 1 - con[X]
        con[HEIGHT] = 1 - con[Y]
    end
    return con
end

-- TODO: improve function name which doesn't representing what it does
function Update_layout(n)
    local i = math.max(math.min(#Layout_data, n), 1)
    print("LAYOUT_DATA0: ", Layout_data)
    print("LAYOUT_DATA1: ", #Layout_data)

    if Update then
      Update(i)
    end
    return Layout_data[i]
end

-- TODO: improve function name not representing what it does
function Update_nmaster(n)
    local i = math.max(math.min(#Master_layout_data, n), 1)
    return Master_layout_data[i]
end
