local function update(n)
    print("update")
    -- if n == 1 then
    --
    --     Resize_direction = 0
    -- end
    -- if n >= 5 then
    --     Resize_direction = Direction.RIGHT + Direction.TOP + Direction.BOTTOM + Direction.LEFT
    -- end
end

local layout_data = {
    {
        -- X, Y, WIDTH, HEIGHT
        {0.0, 0.0, 1, 1},
    },
    {
        {0.0, 0.0, 1.0, 0.8},
        {0.0, 0.8, 1.0, 0.2},
    },
    {
        {0.0, 0.0, 0.8, 0.8},
        {0.0, 0.8, 1.0, 0.2},
        {0.8, 0.0, 0.2, 0.8},
    },
    {
        {0.0, 0.0, 0.8, 0.8},
        {0.0, 0.8, 0.5, 0.2},
        {0.5, 0.8, 0.5, 0.2},
        {0.8, 0.0, 0.2, 0.8},
    },
    {
        {0.2, 0.2, 0.6, 0.6},
        {0.0, 0.0, 0.8, 0.2},
        {0.8, 0.0, 0.2, 0.8},
        {0.2, 0.8, 0.8, 0.2},
        {0.0, 0.2, 0.2, 0.8},
    },
    {
        {0.2, 0.2, 0.6, 0.6},
        {0.0, 0.0, 0.4, 0.2},
        {0.4, 0.0, 0.4, 0.2},
        {0.8, 0.0, 0.2, 0.4},
        {0.8, 0.4, 0.2, 0.4},
        {0.2, 0.8, 0.4, 0.2},
    },
    {
        {0.2, 0.2, 0.6, 0.6},
        {0.0, 0.0, 0.4, 0.2},
        {0.4, 0.0, 0.4, 0.2},
        {0.8, 0.0, 0.2, 0.4},
        {0.8, 0.4, 0.2, 0.4},
        {0.2, 0.8, 0.4, 0.2},
        {0.6, 0.8, 0.4, 0.2},
    },
    {
        {0.2, 0.2, 0.6, 0.6},
        {0.0, 0.0, 0.4, 0.2},
        {0.4, 0.0, 0.4, 0.2},
        {0.8, 0.0, 0.2, 0.4},
        {0.8, 0.4, 0.2, 0.4},
        {0.2, 0.8, 0.4, 0.2},
        {0.6, 0.8, 0.4, 0.2},
        {0.0, 0.2, 0.2, 0.4},
    },
    {
        {0.2, 0.2, 0.6, 0.6},
        {0.0, 0.0, 0.4, 0.2},
        {0.4, 0.0, 0.4, 0.2},
        {0.8, 0.0, 0.2, 0.4},
        {0.8, 0.4, 0.2, 0.4},
        {0.2, 0.8, 0.4, 0.2},
        {0.6, 0.8, 0.4, 0.2},
        {0.0, 0.2, 0.2, 0.4},
        {0.0, 0.6, 0.2, 0.4},
    },
}

local master_layout_data = {
    {
        {0.0, 0.0, 1.0, 1.0}
    },
}

local resize_data = {
    {1},
    {2},
    {3, 4},
    {5},
    {6, 7, 8, 9},
}

layout.set(layout_data, master_layout_data, resize_data)
lconfig.set_resize_direction(Direction.RIGHT + Direction.TOP + Direction.BOTTOM + Direction.LEFT)
lconfig.set_update_function(update)
lconfig.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
lconfig.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
