function Update(n)
end
action.set_resize_direction(Direction.RIGHT)

print("tmp\n")
local layout_data = {
    {
        {0.0, 0.0, 1.0, 1.0},
    },
    {
        {0.0, 0.0, 0.5, 1.0},
        {0.5, 0.0, 0.5, 1.0},
    },
    {
        {0.0, 0.0, 0.5, 1.0},
        {0.5, 0.0, 0.5, 0.5},
        {0.5, 0.5, 0.5, 0.5},
    },
    {
        {0.0, 0.0, 0.5, 1.0},
        {0.5, 0.0, 0.5, 0.333},
        {0.5, 0.333, 0.5, 0.333},
        {0.5, 0.666, 0.5, 0.333},
    },
    {
        {0.0, 0.0, 0.5, 1.0},
        {0.5, 0.0, 0.5, 0.25},
        {0.5, 0.25, 0.5, 0.25},
        {0.5, 0.5, 0.5, 0.25},
        {0.5, 0.75, 0.5, 0.25},
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

local resize_data = {
    {1},
    {2, 3, 4, 5},
}

layout.set(layout_data, master_layout_data, resize_data)
action.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
action.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
