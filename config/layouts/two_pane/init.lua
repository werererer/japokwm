function Update(n)
end
action.set_resize_direction(Direction.LEFT)

local layout_data = {
    {
        {0, 0, 1, 1},
    },
    {
        {0.5, 0.0, 0.5, 1.0},
        {0.0, 0.0, 0.5, 1.0},
    },
}

local master_layout_data = {
    {
        {0.0, 0.0, 1.0, 1.0}
    },
    {
        {0.0, 0.0, 1.0, 0.5},
        {0.0, 0.5, 1.0, 0.5},
    },
}

local box_data = {
    {1},
    {2},
}

layout.set(layout_data, master_layout_data, box_data)
