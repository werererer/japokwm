local layout_data = {
    {
        {0, 0, 1, 1},
    },
    {
        {0.0, 0.0, 0.5, 1.0},
        {0.5, 0.0, 0.5, 1.0},
    },
}

local function update(n)
    if n <= 1 then
        l.config.set_tile_borderpx(0)
    else
        l.config.set_tile_borderpx(2)
    end
end

layout.set("two_pane", layout_data)
l.event.add_listener("on_update", update)
l.config.set_master_layout_data(
{{{0, 0, 1, 1}}, {{0, 0, 0.5, 1}, {0.5, 0, 0.5, 1}}}
)
l.config.set_hidden_edges(info.direction.all)
l.config.set_inner_gaps(0)
l.config.set_outer_gaps(0)
l.config.set_resize_direction(info.direction.right)
l.config.set_layout_constraints({min_width = 0.1, max_width = 1, min_height = 0.1, max_height = 1})
l.config.set_master_constraints({min_width = 0.2, max_width = 1, min_height = 0.2, max_height = 1})
