local X<const> = 1
local Y<const> = 2
local WIDTH<const> = 3
local HEIGHT<const> = 4

-- set: which window conf set
-- client: current window
function Move_container(container, n, d)
    local con = container
    if d == Direction.top then
        con[Y] = con[Y] + n
    elseif d == Direction.bottom then
        con[Y] = con[Y] + n
    elseif d == Direction.left then
        con[X] = con[X] + n
    elseif d == Direction.right then
        con[X] = con[X] + n
    end
    return con
end

-- TODO this is a mess fix it!!
function Is_resize_locked(layout_data_el, o_layout_data_el, i, n, directions)
    local lt_data_el = Deep_copy(layout_data_el)
    local main_con = lt_data_el[i]

    for x = 1,#directions do
        local dir = directions[x]
        local resize_containers = Get_resize_affected_containers(lt_data_el, o_layout_data_el, i, dir, Get_alternative_container, Is_affected_by_resize_of)
        main_con = Deep_copy(Move_resize(main_con, 0, n, dir))
        local alt_con = Get_alternative_container(main_con, dir)

        if info.is_container_not_in_master_limit(main_con) then
            return true
        end

        for k = 1,#resize_containers do
            local lj = resize_containers[k][5]
            local c = lt_data_el[lj]

            c[X] = alt_con[X] + (resize_containers[k][X] * alt_con[WIDTH])
            c[Y] = alt_con[Y] + (resize_containers[k][Y] * alt_con[HEIGHT])
            c[WIDTH] = resize_containers[k][WIDTH] * alt_con[WIDTH]
            c[HEIGHT] = resize_containers[k][HEIGHT] * alt_con[HEIGHT]

            if info.is_container_not_in_limit(c) then
              return true
            end
        end
    end
    return false
end

function Resize_container(container, n, d)
    local con = container
    if d == Direction.top then
        con[Y] = con[Y] - n
        con[HEIGHT] = con[HEIGHT] + n
    elseif d == Direction.bottom then
        con[HEIGHT] = con[HEIGHT] + n
    elseif d == Direction.left then
        con[X] = con[X] - n
        con[WIDTH] = con[WIDTH] + n
    elseif d == Direction.right then
        con[WIDTH] = con[WIDTH] + n
    end
    return con
end

function Move_resize(container, nmove, nresize, d)
    local con = action.deep_copy(container)
    con = Move_container(con, nmove, d)
    con = Resize_container(con, nresize, d)
    return con
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
