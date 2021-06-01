local layout_data = {
    {
        {0, 0, 1, 1},
    },
}

local resize_data = {}

layout.set("monocle", layout_data)
l.config.set_master_layout_data(
{{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}}
)
l.config.set_resize_data(resize_data)
l.config.set_arrange_by_focus(true);
l.config.set_tile_borderpx(0)
l.config.set_float_borderpx(1)
l.config.set_inner_gaps(0)
l.config.set_outer_gaps(0)
