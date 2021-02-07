local layout_data = {
    {
        {0, 0, 1, 1},
    },
    {
        {0.5, 0.0, 0.5, 1.0},
        {0.0, 0.0, 0.5, 1.0},
    },
}

layout.set(layout_data)
lconfig.set_resize_direction(Direction.LEFT)
lconfig.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
lconfig.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
