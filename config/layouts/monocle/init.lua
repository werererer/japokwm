local layout_data = {
    {
        {0, 0, 1, 1},
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
}

layout.set(layout_data, master_layout_data, resize_data)
lconfig.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
lconfig.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
lconfig.set_arrange_by_focus(true);
lconfig.set_tile_borderpx(0)
lconfig.set_float_borderpx(1)
lconfig.set_gaps(0, 0)
