require "tile"

Cursor_mode = {
    CUR_NORMAL = 0,
    CUR_MOVE = 1,
    CUR_RESIZE = 2,
}

-- TODO complete transformations
Monitor_transformation = {
    NORMAL = 0,
}

Layout_id = 1

function Set_layout()
    Layout_id = Layout_id + 1
    if Layout_id > #Layouts then
        Layout_id = 1
    end
    action.arrange()
end

function Set_layout(i)
    Layout_id = i
    local layout = Layouts[i]
    local layout_name = layout[2]
    Load_layout(layout_name)
    action.arrange()
end
