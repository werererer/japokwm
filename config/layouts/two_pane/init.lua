local layout_data = {
    {
        {0, 0, 1, 1},
    },
    {
        {0.0, 0.0, 0.5, 1.0},
        {0.5, 0.0, 0.5, 1.0},
    },
}

local function update(layout)
end

local resize_data = {
    {
        {1, 2, 3, 4, 5, 6}
    },
    {
        {1, 2, 3, 4, 5, 6}
    },
}

layout:set(layout_data)
layout:set_resize_data(resize_data)
event:add_listener("on_update", update)
layout:set_linked_layouts({"three_columns"})
layout:set_master_layout_data(
{{{0, 0, 1, 1}}, {{0, 0, 1, 0.5}, {0, 0.5, 1, 0.5}}}
)
opt.hidden_edges = Direction.all
opt.inner_gaps = 0
opt.outer_gaps = 0
opt.resize_direction = Direction.right
opt:set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
opt:set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
