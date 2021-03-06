local layout_data = {
    {
        {0, 0, 1, 1},
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
        {0.0, 0.000, 0.5, 1.000},
        {0.5, 0.000, 0.5, 0.333},
        {0.5, 0.333, 0.5, 0.333},
        {0.5, 0.666, 0.5, 0.333},
    },
    {
        {0.0, 0.00, 0.5, 1.00},
        {0.5, 0.00, 0.5, 0.25},
        {0.5, 0.25, 0.5, 0.25},
        {0.5, 0.50, 0.5, 0.25},
        {0.5, 0.75, 0.5, 0.25},
    },
}

layout.set("tile", layout_data)
l.config.set_hidden_edges(info.direction.all)
l.config.set_smart_hidden_edges(false)
l.config.set_resize_direction(info.direction.right)
l.config.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
l.config.set_master_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
