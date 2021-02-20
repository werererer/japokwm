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
l.config.set_inner_gaps(0)
l.config.set_outer_gaps(0)
l.config.set_resize_direction(info.direction.left)
l.config.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
l.config.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
