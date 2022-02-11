require "tileutils"

local X<const> = 1
local Y<const> = 2
local WIDTH<const> = 3
local HEIGHT<const> = 4

function round(num, n)
    local mult = 10^(n or 0)
    return math.floor(num * mult + 0.5) / mult
end

function round_2(num)
    return round(num, 2)
end

-- val is between 0 and 1 and represents how far
local function abs_container_to_relative(con, ref_area)
    con[X] = con[X] / ref_area[WIDTH] - ref_area[X]
    con[Y] = con[Y] / ref_area[HEIGHT] - ref_area[Y]
    con[WIDTH] = con[WIDTH] / ref_area[WIDTH]
    con[HEIGHT] = con[HEIGHT] / ref_area[HEIGHT]
end

local function relative_container_to_abs(con, ref_area)
    con[X] = con[X] * ref_area[WIDTH] + ref_area[X]
    con[Y] = con[Y] * ref_area[HEIGHT] + ref_area[Y]
    con[WIDTH] = con[WIDTH] * ref_area[WIDTH]
    con[HEIGHT] = con[HEIGHT] * ref_area[HEIGHT]
end

local function create_container(x, y, width, height)
    local con = {}
    con[X] = x
    con[Y] = y
    con[WIDTH] = width
    con[HEIGHT] = height
    return con
end

local function get_area_at_zero(area)
    local x = Gmp.new(0)
    local y = Gmp.new(0)
    local width = area[WIDTH]
    local height = area[HEIGHT]
    local con = create_container(x, y, width, height)
    return con
end

local function get_area_local_con(con, area)
    local x1 = con[X] - area[X]
    local y1 = con[Y] - area[Y]
    local x2 = con[X] + con[WIDTH] - area[X]
    local y2 = con[Y] + con[HEIGHT] - area[Y]
    local local_con = create_container(x1, y1, x2-x1, y2-y1)
    return local_con
end

local function get_global_con(con, area)
    local x1 = con[X] + area[X]
    local y1 = con[Y] + area[Y]
    local x2 = con[X] + con[WIDTH] + area[X]
    local y2 = con[Y] + con[HEIGHT] + area[Y]
    local local_con = create_container(x1, y1, x2-x1, y2-y1)
    return local_con
end

local function assign_con_values(con, new_con)
    con[X] = new_con[X]
    con[Y] = new_con[Y]
    con[WIDTH] = new_con[WIDTH]
    con[HEIGHT] = new_con[HEIGHT]
end

local function change_base(con, old_area, new_area)
    local local_old_con = get_area_local_con(con, old_area);
    local zero_old_area = get_area_at_zero(old_area)
    local zero_new_area = get_area_at_zero(new_area)
    abs_container_to_relative(local_old_con, zero_old_area)
    relative_container_to_abs(local_old_con, zero_new_area)

    local new_con = get_global_con(local_old_con, new_area)
    assign_con_values(con, new_con)
end

local function merge_boxes(box1, box2)
    if (box1 == nil) then
        return Action.deep_copy(box2)
    end
    if (box2 == nil) then
        return Action.deep_copy(box1)
    end
    local x1 = Gmp.min(box1[X], box2[X])
    local y1 = Gmp.min(box1[Y], box2[Y])
    local x2 = Gmp.max(box1[X] + box1[WIDTH], box2[X] + box2[WIDTH])
    local y2 = Gmp.max(box1[Y] + box1[HEIGHT], box2[Y] + box2[HEIGHT])

    local con = create_container(x1, y1, x2 - x1, y2 - y1)
    return con
end

local function join_containers(con1, con2, con3)
    local res_container = merge_boxes(con1, con2)
    res_container = merge_boxes(res_container, con3)
    return res_container
end

local function intersection_of(con1, con2)
    local x1 = Gmp.max(con1[X], con2[X])
    local y1 = Gmp.max(con1[Y], con2[Y])
    local x2 = Gmp.min(con1[X] + con1[WIDTH], con2[X] + con2[WIDTH])
    local y2 = Gmp.min(con1[Y] + con1[HEIGHT], con2[Y] + con2[HEIGHT])
    local con = create_container(x1, y1, x2 - x1, y2 - y1)

    if (con[WIDTH] <= Gmp.new(0)) then
        return nil
    end
    if (con[HEIGHT] <= Gmp.new(0)) then
        return nil
    end
    return con
end

local function split_container(con, unaffected_area, old_alpha_area, old_beta_area)
    local non_affected_con = intersection_of(con, unaffected_area)
    local alpha_con = intersection_of(con, old_alpha_area)
    local beta_con = intersection_of(con, old_beta_area)
    return non_affected_con, alpha_con, beta_con
end

local function create_gmp_container(con)
    local new_con = {}
    new_con[X] = Gmp.new(con[X])
    new_con[Y] = Gmp.new(con[Y])
    new_con[WIDTH] = Gmp.new(con[WIDTH])
    new_con[HEIGHT] = Gmp.new(con[HEIGHT])
    return new_con
end

local function apply_resize(lt_data_el, old_unaffected_area, old_alpha_area, new_alpha_area, old_beta_area, new_beta_area)
    -- local i = 5
    for i = 1,#lt_data_el do
        local con = create_gmp_container(lt_data_el[i]);
        local unaffected_con, alpha_con, beta_con = split_container(con, old_unaffected_area, old_alpha_area, old_beta_area);

        if (alpha_con ~= nil) then
            change_base(alpha_con, old_alpha_area, new_alpha_area)
        end
        if (beta_con ~= nil) then
            change_base(beta_con, old_beta_area, new_beta_area)
        end

        local new_con = join_containers(unaffected_con, alpha_con, beta_con)
        local lt_con = lt_data_el[i]
        lt_con[X] = new_con[X].value
        lt_con[Y] = new_con[Y].value
        lt_con[WIDTH] = new_con[WIDTH].value
        lt_con[HEIGHT] = new_con[HEIGHT].value
    end
end

local function get_cissor_container_left(alpha_area)
    local x1 = Gmp.new(0)
    local y1 = Gmp.new(0)
    local x2 = alpha_area[X]
    local y2 = Gmp.new(1)
    local con = create_container(x1, y1, x2-x1, y2-y1)
    return con
end

local function get_cissor_container_top(alpha_area)
    local x1 = Gmp.new(0)
    local y1 = Gmp.new(0)
    local x2 = Gmp.new(1)
    local y2 = alpha_area[Y]
    local con = create_container(x1, y1, x2-x1, y2-y1)
    return con
end

local function get_cissor_container_bottom(alpha_area)
    local x1 = Gmp.new(0)
    local y1 = alpha_area[Y] + alpha_area[HEIGHT]
    local x2 = Gmp.new(1)
    local y2 = Gmp.new(1)
    local con = create_container(x1, y1, x2-x1, y2-y1)
    return con
end

local function get_cissor_container_right(alpha_area)
    local x1 = alpha_area[X] + alpha_area[WIDTH]
    local y1 = Gmp.new(0)
    local x2 = Gmp.new(1)
    local y2 = Gmp.new(1)
    local con = create_container(x1, y1, x2-x1, y2-y1)
    return con
end

local function get_opposite_direction(dir)
    if dir == Direction.left then
        return Direction.right
    elseif dir == Direction.right then
        return Direction.left
    elseif dir == Direction.top then
        return Direction.bottom
    elseif dir == Direction.bottom then
        return Direction.top
    end
end

local function get_cissor_container(alpha_area, dir)
    local area = nil
    if dir == Direction.left then
        area = get_cissor_container_left(alpha_area)
    elseif dir == Direction.right then
        area = get_cissor_container_right(alpha_area)
    elseif dir == Direction.top then
        area = get_cissor_container_top(alpha_area)
    elseif dir == Direction.bottom then
        area = get_cissor_container_bottom(alpha_area)
    end
    return area
end

local function get_beta_area(alpha_area, dir)
    local area = get_cissor_container(alpha_area, dir)
    return area
end

local function get_unaffected_area(old_alpha_area, dir)
    local opposite_direction = get_opposite_direction(dir)
    local area = get_beta_area(old_alpha_area, opposite_direction)
    return area
end

local function get_alpha_container_horizontal(con)
    local x1 = con[X]
    local y1 = Gmp.new(0)
    local x2 = con[X] + con[WIDTH]
    local y2 = Gmp.new(1)
    local area = create_container(x1, y1, x2-x1, y2-y1)
    return area
end

local function get_alpha_container_vertical(con)
    local x1 = Gmp.new(0)
    local y1 = con[Y]
    local x2 = Gmp.new(1)
    local y2 = con[Y] + con[HEIGHT]
    local area = create_container(x1, y1, x2-x1, y2-y1)
    return area
end

local function get_alpha_area_from_container(con, dir)
    local area = nil
    if dir == Direction.left or dir == Direction.right then
        area = get_alpha_container_horizontal(con)
    elseif dir == Direction.top or dir == Direction.bottom then
        area = get_alpha_container_vertical(con)
    end
    return area
end

local function is_invalid(con)
    if con[WIDTH] < Gmp.new(0) then
        return true
    end
    if con[HEIGHT] < Gmp.new(0) then
        return true
    end
    return false
end

-- return an array of the new directions
local function get_transform_directions(con, new_geom)
    local directions = {}
    -- left
    if new_geom[X] ~= con[X] then
        directions[#directions+1] = Direction.left
    end
    -- right
    if new_geom[WIDTH] ~= con[WIDTH] then
        directions[#directions+1] = Direction.right
    end
    -- top
    if new_geom[Y] ~= con[Y] then
        directions[#directions+1] = Direction.top
    end
    -- bottom
    if new_geom[HEIGHT] ~= con[HEIGHT] then
        directions[#directions+1] = Direction.bottom
    end
    return directions
end

local function apply_resize_function_with_geometry(lt_data_el, i, new_geom)
    if i > #lt_data_el then
        return
    end
    local old_geom = create_gmp_container(lt_data_el[i])
    local transform_directions = get_transform_directions(old_geom, new_geom)
    for x = 1,#transform_directions do
        local dir = transform_directions[x]

        local old_alpha_area = get_alpha_area_from_container(old_geom, dir)
        local new_alpha_area = get_alpha_area_from_container(new_geom, dir)

        local old_beta_area = get_beta_area(old_alpha_area, dir)
        local new_beta_area = get_beta_area(new_alpha_area, dir)
        local old_unaffected_area = get_unaffected_area(old_alpha_area, dir)

        if is_invalid(new_alpha_area) or is_invalid(new_beta_area) then
            return
        end

        apply_resize(lt_data_el, old_unaffected_area, old_alpha_area, new_alpha_area, old_beta_area, new_beta_area)
    end
end

local function apply_resize_function(lt_data_el, i, n, directions)
    local old_geom = lt_data_el[i]
    for x = 1,#directions do
        local dir = directions[x]

        local old_alpha_area = get_alpha_area_from_container(old_geom, dir)
        local new_alpha_area = Move_resize(old_alpha_area, 0, n, dir)

        local old_beta_area = get_beta_area(old_alpha_area, dir)
        local new_beta_area = get_beta_area(new_alpha_area, dir)
        local old_unaffected_area = get_unaffected_area(old_alpha_area, dir)

        if is_invalid(new_alpha_area) or is_invalid(new_beta_area) then
            return
        end

        apply_resize(lt_data_el, old_unaffected_area, old_alpha_area, new_alpha_area, old_beta_area, new_beta_area)
    end
end

-- TODO refactor and simplify
local function resize_all(lt_data_el, i, n, d)
    if i > #lt_data_el then
        return lt_data_el
    end

    local directions = Get_directions(d)
    local layout_data_element = Action.deep_copy(lt_data_el)

    -- if Is_resize_locked(layout_data_element, o_layout_data_el, i, n, directions) then
    --     return layout_data_element
    -- end

    apply_resize_function(layout_data_element, i, n, directions)

    return layout_data_element
end


local function get_layout_data_element_id(o_layout_data)
    return math.max(math.min(Info.get_this_container_count(), #o_layout_data), 1)
end

-- returns 0 if not found
local function get_layout_element(layout_data_element_id, resize_data)
    for j=1,#resize_data do
        local resize_data_element = resize_data[j]
        for h=1, #resize_data[j] do
            if layout_data_element_id == resize_data_element[h] then
                return j
            end
        end
    end
    return 0
end

local function transform_direction_get_directions(transform_direction)
    local directions = 0
    if transform_direction[1] ~= 0 then
        directions = directions + Direction.left
    end
    if transform_direction[2] ~= 0 then
        directions = directions + Direction.right
    end
    if transform_direction[3] ~= 0 then
        directions = directions + Direction.top
    end
    if transform_direction[4] ~= 0 then
        directions = directions + Direction.bottom
    end
    return directions
end

-- i: position of the element inside the layout
function Resize_container_in_layout(lt, i, new_geom)
    local layout_data_element_id = get_layout_data_element_id(lt.o_layout_data)
    local layout_id = get_layout_element(layout_data_element_id, lt.resize_data)
    if layout_id <= 0 then
        return lt.layout_data
    end
    if i <= 0 then
        return lt.layout_data
    end

    local new_lua_geom = {}
    new_lua_geom[X] = Gmp.new(new_geom.x)
    new_lua_geom[Y] = Gmp.new(new_geom.y)
    new_lua_geom[WIDTH] = Gmp.new(new_geom.width)
    new_lua_geom[HEIGHT] = Gmp.new(new_geom.height)

    local mon_geom = Monitor.focused.root.geom
    local mon_geom_transfered = {}
    mon_geom_transfered[X] = Gmp.new(mon_geom.x)
    mon_geom_transfered[Y] = Gmp.new(mon_geom.y)
    mon_geom_transfered[WIDTH] = Gmp.new(mon_geom.width)
    mon_geom_transfered[HEIGHT] = Gmp.new(mon_geom.height)
    print("prev: ", (new_lua_geom[X]+new_lua_geom[WIDTH])/mon_geom_transfered[WIDTH])
    abs_container_to_relative(new_lua_geom, mon_geom_transfered)
    print("new: ", new_lua_geom[X]+new_lua_geom[WIDTH])

    local resize_element = lt.resize_data[layout_id]
    for _,id in ipairs(resize_element) do
        if id <= #lt.o_layout_data then
            apply_resize_function_with_geometry(lt.layout_data[id], i, new_lua_geom)
        end
    end
    return lt.layout_data
end

function Resize_main_all(lt, n, direction)
    local layout_data_element_id = get_layout_data_element_id(lt.o_layout_data)
    local layout_id = get_layout_element(layout_data_element_id, lt.resize_data)
    if layout_id == 0 then
        return lt.layout_data
    end

    local resize_element = lt.resize_data[layout_id]
    for _,id in ipairs(resize_element) do
        if id <= #lt.o_layout_data then
            lt.layout_data[id] = resize_all(lt.layout_data[id], 1, n, direction)
        end
    end
    return lt.layout_data
end
