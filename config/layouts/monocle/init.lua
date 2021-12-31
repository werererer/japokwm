local layout_data = {
    {
        {0, 0, 1, 1},
    },
}

local resize_data = {}

layout:set(layout_data)
layout:set_master_layout_data({{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}})

layout:set_resize_data(resize_data)
opt.arrange_by_focus = true;
opt.border_width = 0
opt.float_border_width = 1
opt.inner_gaps = 0
opt.outer_gaps = 0
