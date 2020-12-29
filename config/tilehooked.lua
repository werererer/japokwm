require "tileutils"

Resize_direction = Direction.BOTTOM

local X<const> = 1
local Y<const> = 2
local WIDTH<const> = 3
local HEIGHT<const> = 4

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

function Is_equally_affected_by_resize_of_hooked(container, container2, d)
    local resize = false
    if d == Direction.TOP then
        resize = Is_approx_equal(container2[Y], container[Y])
    elseif d == Direction.BOTTOM then
        resize = Is_approx_equal(container2[Y] + container2[HEIGHT], container[Y] + container[HEIGHT])
    elseif d == Direction.LEFT then
        resize = Is_approx_equal(container2[X], container[X])
    elseif d == Direction.RIGHT then
        resize = Is_approx_equal(container2[X] + container2[WIDTH], container[X] + container[WIDTH])
    end
    return resize
end

function Resize_all_hooked(i, j, n, d)
    local directions = Get_directions(d)
    local container = Layout_data[i][j]

    local lock = false
    -- check
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

    if lock then
      return
    end

    -- apply
    for x = 1,#directions do
        local dir = directions[x]
        local resize_main_containers = Get_resize_affected_containers(i, j, dir, Get_main_container, Is_equally_affected_by_resize_of_hooked)
        local resize_containers = Get_resize_affected_containers(i, j, dir, Get_alternative_container, Is_affected_by_resize_of)
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
            Layout_data[li][lj],lock = Move_resize(Layout_data[li][lj], 0, n, dir)
            if lock then
                print("locked")
            end
        end
    end
end

function Resize_main_all_hooked(n, d)
    local i = math.max(math.min(info.this_tiled_client_count(), #Layout_data), 1)

    for g=1,#Box_data do
        for h=1,#Box_data[g] do
            if i == Box_data[g][h] then
                for j=1,#Box_data[g] do
                    Resize_all_hooked(Box_data[g][j], 1, n, d)
                    action.arrange_this(false)
                end
                break
            end
        end
    end
end
