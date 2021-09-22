local layout_data = {
    {
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

local resize_data = {
    {1},
    {2},
    {3, 4},
    {5},
    {6, 7, 8, 9},
}

layout.set(layout_data)
l.config.set_resize_data(resize_data)
l.config.set_master_layout_data(
{{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}}
)
l.config.set_resize_direction(info.direction.right + info.direction.top + info.direction.bottom + info.direction.left)
l.config.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
l.config.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
