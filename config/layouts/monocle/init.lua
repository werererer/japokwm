function Update(n)
    print("update: ", n)
end

action.set_resize_direction(0)
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

local box_data = {
    {1},
}

layout.set(layout_data, master_layout_data, box_data)
action.set_arrange_by_focus(true);
lconfig.set_borderpx(0)
lconfig.set_gaps(0, 0)
