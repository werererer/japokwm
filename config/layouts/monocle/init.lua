local layout_data = {
    {
        {0, 0, 1, 1},
    },
}

local resize_data = {}

layout:set(layout_data)
opt:set_master_layout_data({{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}})

opt:resize_data = resize_data
opt:set_arrange_by_focus(true);
opt:set_tile_borderpx(0)
opt:set_float_borderpx(1)
opt:set_inner_gaps(0)
opt:set_outer_gaps(0)
