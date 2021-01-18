require "tileutils"

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
        {0, 0, 1, 1},
    },
    {
        {0.5, 0.0, 0.5, 1.0},
        {0.0, 0.0, 0.5, 1.0},
    },
}

Master_layout_data = {
    {
        {0, 0, 1.0, 1},
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

function Is_equally_affected_by_resize_of(container, container2, d)
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

function Resize_this_all(n, d)
    local i = math.max(math.min(info.get_this_container_count(), #Layout_data), 1)
    local j = math.min(info.this_container_position(), #Layout_data[i])
    Resize_all(Layout_data, i, j, n, d)
    action.arrange_this(false)
end

-- finds containers that are affected by the container at i,j
function Get_resize_affected_containers(layout_data, o_layout_data, i, j, d, get_container_func, is_effected_by_func)
    local container = layout_data[i][j]
    local list = {}

    for j2 = 1, #layout_data[i] do
        local con = layout_data[i][j2]
        local alt_con = get_container_func(container, d)

        if j ~= j2 then
            if is_effected_by_func(o_layout_data[i][j], o_layout_data[i][j2], d) then
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

function Move_this_container(n, d)
    local i = math.max(math.min(action.get_this_container_count(), #Layout_data), 1)
    local j = math.min(info.this_container_position(), #Layout_data[i])
    local container = Layout_data[i][j]
    Layout_data[i][j] = Move_container(container, n, d)
    action.arrange_this(false)
end

function Resize_this_container(n, d)
    local i = math.max(math.min(action.get_this_container_count(), #Layout_data), 1)
    local j = math.min(action.client_pos(), #Layout_data[i])
    Layout_data[i][j] = Resize_container(Layout_data[i][j], n, d)
    action.arrange_this(false)
end

--
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

function Resize_all(lt_data, o_layout_data, i, j, n, d)
    local directions = Get_directions(d)
    local layout_data = Deep_copy(lt_data)
    local container = layout_data[i][j]

    if Is_resize_locked(layout_data, o_layout_data, i, j, n, directions) then
        return layout_data
    end

    -- apply
    for x = 1,#directions do
        local dir = directions[x]
        local resize_main_containers = Get_resize_affected_containers(layout_data, o_layout_data, i, j, dir, Get_main_container, Is_equally_affected_by_resize_of)
        local resize_containers = Get_resize_affected_containers(layout_data, o_layout_data, i, j, dir, Get_alternative_container, Is_affected_by_resize_of)
        local main_con = Move_resize(container, 0, n, dir)
        local alt_con = Get_alternative_container(main_con, dir)

        for k = 1,#resize_containers do
            local li = resize_containers[k][5]
            local lj = resize_containers[k][6]

            layout_data[li][lj][X] = alt_con[X] + (resize_containers[k][X] * alt_con[WIDTH])
            layout_data[li][lj][Y] = alt_con[Y] + (resize_containers[k][Y] * alt_con[HEIGHT])
            layout_data[li][lj][WIDTH] = resize_containers[k][WIDTH] * alt_con[WIDTH]
            layout_data[li][lj][HEIGHT] = resize_containers[k][HEIGHT] * alt_con[HEIGHT]
        end

        layout_data[i][j][X] = main_con[X]
        layout_data[i][j][Y] = main_con[Y]
        layout_data[i][j][WIDTH] = main_con[WIDTH]
        layout_data[i][j][HEIGHT] = main_con[HEIGHT]
        for k = 1,#resize_main_containers do
            local li = resize_main_containers[k][5]
            local lj = resize_main_containers[k][6]
            layout_data[li][lj] = Move_resize(layout_data[li][lj], 0, n, dir)
        end
    end
    return layout_data
end

function Resize_main_all(layout_data, o_layout_data, box_data, n, d)
    local i = math.max(math.min(info.get_this_container_count(), #o_layout_data), 1)
    for g=1,#box_data do
        for h=1,#box_data[g] do
            if i == box_data[g][h] then
                for j=1,#box_data[g] do
                    layout_data = Resize_all(layout_data, o_layout_data, box_data[g][j], 1, n, d)
                end
                break
            end
        end
    end
    return layout_data
end
